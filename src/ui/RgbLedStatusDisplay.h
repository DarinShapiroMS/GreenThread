#pragma once
#include "StatusDisplay.h"
#include <Arduino.h>

// Hardware pin definitions (Arduino Nano Matter)
constexpr uint8_t PIN_R = 23;  // RED is pin 23 (active LOW)
constexpr uint8_t PIN_G = 24;  // GREEN is pin 24 (active LOW)  
constexpr uint8_t PIN_B = 22;  // BLUE is pin 22 (active LOW)

// Timing constants - all discoverable at top of file
constexpr uint16_t BOOT_HOLD_MS = 1000;           // Boot green hold duration
constexpr uint16_t BATTERY_FLASH_MS = 200;        // Battery warning flash duration
constexpr uint16_t BLINK_DURATION_MS = 400;       // Moisture blink on/off duration
constexpr uint16_t FAILURE_BLINK_MS = 1000;       // Connection failure blink duration
constexpr uint8_t MOISTURE_BLINK_COUNT = 5;       // Number of moisture indication blinks

// Commissioning timing constants
constexpr uint16_t COMMISSIONING_FAST_BLINK_MS = 250;   // Fast white blink when ready
constexpr uint16_t COMMISSIONING_SLOW_BLINK_MS = 800;   // Slow green blink when in progress
constexpr uint16_t COMMISSIONING_SUCCESS_HOLD_MS = 3000; // Solid green for success
constexpr uint32_t COMMISSIONING_TIMEOUT_MS = 180000;   // 3 minutes timeout

// Debug control - disable to remove all strings from flash
enum class DebugLevel : uint8_t { DISABLED = 0, ENABLED = 1 };
constexpr DebugLevel LED_DEBUG = DebugLevel::ENABLED;

// Conditional logging macros - strings removed in release builds
#define LOG_LED(msg) do { \
  if constexpr (LED_DEBUG == DebugLevel::ENABLED) { \
    Serial.println(F("[RGB LED] " msg)); \
  } \
} while(0)

#define LOG_LED_STATE(msg, state) do { \
  if constexpr (LED_DEBUG == DebugLevel::ENABLED) { \
    Serial.print(F("[RGB LED] " msg)); \
    Serial.println((int)state); \
  } \
} while(0)

// Compact color representation using PROGMEM lookup table
enum class ColorIndex : uint8_t {
  OFF = 0,     // Black
  RED = 1,     // Driest
  ORANGE = 2,  // Very dry
  YELLOW = 3,  // Slightly dry
  WHITE = 4,   // Middle quartile
  GREEN = 5,   // Slightly wet & boot
  BLUE = 6     // Wettest
};

// PROGMEM color table - saves RAM by storing in flash
struct RGBValues {
  bool r, g, b;
  constexpr RGBValues(bool red = false, bool green = false, bool blue = false) : r(red), g(green), b(blue) {}
};

static const RGBValues PROGMEM COLOR_TABLE[] = {
  {false, false, false},  // OFF
  {true, false, false},   // RED
  {true, true, false},    // ORANGE
  {true, true, false},    // YELLOW (same as orange for RGB LED)
  {true, true, true},     // WHITE
  {false, true, false},   // GREEN
  {false, false, true}    // BLUE
};

// Moisture percentage thresholds for color lookup (ascending order for binary search)
static const uint8_t PROGMEM MOISTURE_THRESHOLDS[] = {10, 25, 40, 55, 70, 85};

struct RGBColor {
  ColorIndex index;
  constexpr RGBColor(ColorIndex idx = ColorIndex::OFF) : index(idx) {}
  constexpr bool operator==(const RGBColor& other) const {
    return index == other.index;
  }
  
  // Get RGB values from PROGMEM table
  RGBValues getRGB() const {
    RGBValues result;
    memcpy_P(&result, &COLOR_TABLE[static_cast<uint8_t>(index)], sizeof(RGBValues));
    return result;
  }
};

// Generic blinker for unified blink logic
struct Blinker {
  bool phase = false;
  uint32_t nextTime = 0;
  uint16_t period = 0;
  uint8_t count = 0;
  uint8_t current = 0;
  
  bool ready(uint32_t now) const { return now >= nextTime; }
  
  void start(uint32_t now, uint16_t periodMs, uint8_t blinkCount = 0) {
    phase = false;
    nextTime = now;
    period = periodMs;
    count = blinkCount;
    current = 0;
  }
  
  bool flip(uint32_t now) {
    phase = !phase;
    nextTime = now + period;
    if (!phase) current++;  // Count full cycles (when going from ON to OFF)
    return (count == 0) || (current < count); // Continue if infinite or not done
  }
};

enum class LEDState : uint8_t {
  OFF,               // Most of the time
  BOOT_GREEN,        // Green during boot with timeout
  MOISTURE_BLINKING, // Color blinks when showing moisture
  CONNECTION_FAILURE, // Slow red blink for failures
  // Commissioning states - structured for future expansion
  COMMISSIONING_READY,    // Fast white blink - ready to commission
  COMMISSIONING_ACTIVE,   // Slow green blink - commissioning in progress
  COMMISSIONING_SUCCESS,  // Solid green hold - commissioning succeeded
  COMMISSIONING_FAILED,   // Fast red blink - commissioning failed
  TEST_MODE          // Manual test override - disable state machine
};

class RgbLedStatusDisplay : public StatusDisplay {
public:
  // Constructor initializes blinkers for safety
  RgbLedStatusDisplay() : moistureBlinker{}, failureBlinker{}, batteryBlinker{}, commissioningBlinker{} {}
  
  void begin() override;
  void handleEvent(StatusEvent event) override;
  void showMoisture(float percent) override;
  void showMessage(const char* msg) override;
  void update() override;
  
  // Unified test method for debugging LED hardware
  void testColor(bool r, bool g, bool b);
  void testRed() { testColor(true, false, false); }
  void testGreen() { testColor(false, true, false); }
  void testBlue() { testColor(false, false, true); }
  void testOff() { testColor(false, false, false); }

private:
  // Inline constexpr setColor for performance - bounds checked
  inline void setColor(bool r, bool g, bool b) {
    // Bounds check: ensure only valid pin states
    r = r ? true : false;  // Normalize to boolean
    g = g ? true : false;
    b = b ? true : false;
    
    digitalWrite(PIN_R, r ? LOW : HIGH);  // Active LOW
    digitalWrite(PIN_G, g ? LOW : HIGH);
    digitalWrite(PIN_B, b ? LOW : HIGH);
  }
  
  inline void setColor(const RGBColor& color) {
    RGBValues rgb = color.getRGB();
    setColor(rgb.r, rgb.g, rgb.b);
  }
  
  // Binary search for efficient moisture color lookup
  ColorIndex getMoistureColorIndex(float percent);
  
  bool isInitialized = false;    // Safety flag - prevents garbage output
  LEDState currentState = LEDState::OFF;
  uint32_t stateStartTime = 0;   // When current state began
  
  // Separate blinkers to prevent conflicts
  Blinker moistureBlinker;
  Blinker failureBlinker;
  Blinker batteryBlinker;        // Dedicated battery blinker
  Blinker commissioningBlinker;  // Dedicated commissioning blinker
  RGBColor blinkColor;           // Current color for moisture blinks
};
