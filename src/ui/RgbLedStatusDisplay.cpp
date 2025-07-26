#include "RgbLedStatusDisplay.h"
#include <Arduino.h>

void RgbLedStatusDisplay::begin() {
  LOG_LED("Initializing LED display...");
  
  // Ensure LED is off before setting pin modes (power optimization)
  digitalWrite(PIN_R, HIGH);  // Active LOW - ensure off
  digitalWrite(PIN_G, HIGH);
  digitalWrite(PIN_B, HIGH);
  
  pinMode(PIN_R, OUTPUT);  // Red
  pinMode(PIN_G, OUTPUT);  // Green  
  pinMode(PIN_B, OUTPUT);  // Blue
  
  currentState = LEDState::OFF;
  setColor(false, false, false);
  isInitialized = true;  // Enable update() processing
  LOG_LED("LED off - ready");
}

void RgbLedStatusDisplay::handleEvent(StatusEvent event) {
  LOG_LED_STATE("Event received", (int)event);
  
  // Don't process events if in test mode
  if (currentState == LEDState::TEST_MODE) {
    LOG_LED("IGNORING EVENT - IN TEST MODE");
    return;
  }
  
  uint32_t now = millis();
  
  switch (event) {
    case StatusEvent::BootStarting:
      LOG_LED("BOOT STARTING - setting green with timeout");
      currentState = LEDState::BOOT_GREEN;
      moistureBlinker.start(now, BOOT_HOLD_MS, 1);  // Single "blink" for timeout
      setColor(RGBColor(ColorIndex::GREEN));
      break;
      
    case StatusEvent::BootSensorInit:
    case StatusEvent::BootNetworkInit:
    case StatusEvent::BootMatterInit:
      // Keep green LED on during boot phases
      if (currentState != LEDState::BOOT_GREEN) {
        LOG_LED("Boot phase - ensuring green is on");
        currentState = LEDState::BOOT_GREEN;
        setColor(RGBColor(ColorIndex::GREEN));
      }
      break;
      
    case StatusEvent::BootComplete:
      LOG_LED("BOOT COMPLETE - turning off");
      currentState = LEDState::OFF;
      setColor(RGBColor(ColorIndex::OFF));
      break;
    
    case StatusEvent::ThreadConnectionFailed:
    case StatusEvent::MatterConnectionFailed:
    case StatusEvent::Error:
      if (currentState != LEDState::CONNECTION_FAILURE) {
        LOG_LED("CONNECTION FAILURE - starting red blink");
        currentState = LEDState::CONNECTION_FAILURE;
        failureBlinker.start(now, FAILURE_BLINK_MS);
        setColor(RGBColor(ColorIndex::RED));
      }
      break;
    
    case StatusEvent::ThreadConnected:
    case StatusEvent::MatterOnline:
      if (currentState == LEDState::CONNECTION_FAILURE) {
        LOG_LED("CONNECTION RESTORED - turning off");
        currentState = LEDState::OFF;
        setColor(RGBColor(ColorIndex::OFF));
      }
      break;
    
    case StatusEvent::ThreadDisconnected:
    case StatusEvent::MatterOffline:
      LOG_LED("Disconnection noted - waiting for failure event");
      break;
    
    case StatusEvent::EnteringSleep:
      LOG_LED("FORCE SLEEP - stopping all blinks");
      // Force everything off immediately - no exceptions
      currentState = LEDState::OFF;
      moistureBlinker.count = 0; // Stop any ongoing moisture blinks
      failureBlinker.count = 0;  // Stop any ongoing failure blinks  
      batteryBlinker.count = 0;  // Stop battery blinks
      setColor(RGBColor(ColorIndex::OFF));
      break;
    
    case StatusEvent::BatteryLow:
      LOG_LED("BATTERY LOW - starting flash sequence");
      // Use dedicated battery blinker to prevent conflicts
      batteryBlinker.start(now, BATTERY_FLASH_MS, 1);
      blinkColor = RGBColor(ColorIndex::RED);
      break;
      
    // Commissioning events - future-proof structure
    case StatusEvent::CommissioningButtonPressed:
    case StatusEvent::CommissioningModeActive:
      LOG_LED("COMMISSIONING MODE - fast white blink");
      currentState = LEDState::COMMISSIONING_READY;
      stateStartTime = now;
      commissioningBlinker.start(now, COMMISSIONING_FAST_BLINK_MS);
      setColor(RGBColor(ColorIndex::WHITE));
      break;
      
    case StatusEvent::CommissioningInProgress:
      LOG_LED("COMMISSIONING ACTIVE - slow green blink");
      currentState = LEDState::COMMISSIONING_ACTIVE;
      stateStartTime = now;
      commissioningBlinker.start(now, COMMISSIONING_SLOW_BLINK_MS);
      setColor(RGBColor(ColorIndex::GREEN));
      break;
      
    case StatusEvent::CommissioningSuccess:
      LOG_LED("COMMISSIONING SUCCESS - solid green hold");
      currentState = LEDState::COMMISSIONING_SUCCESS;
      stateStartTime = now;
      commissioningBlinker.start(now, COMMISSIONING_SUCCESS_HOLD_MS, 1); // Single timeout
      setColor(RGBColor(ColorIndex::GREEN));
      break;
      
    case StatusEvent::CommissioningFailed:
    case StatusEvent::CommissioningTimeout:
      LOG_LED("COMMISSIONING FAILED - fast red blink");
      currentState = LEDState::COMMISSIONING_FAILED;
      stateStartTime = now;
      commissioningBlinker.start(now, COMMISSIONING_FAST_BLINK_MS, 10); // 10 fast blinks then off
      setColor(RGBColor(ColorIndex::RED));
      break;
      
    case StatusEvent::FactoryReset:
      LOG_LED("FACTORY RESET - returning to OFF");
      currentState = LEDState::OFF;
      setColor(RGBColor(ColorIndex::OFF));
      break;
      
    default:
      LOG_LED("Event ignored");
      break;
  }
}

void RgbLedStatusDisplay::showMoisture(float percent) {
  LOG_LED("Moisture reading - starting blink sequence");
  
  ColorIndex colorIndex = getMoistureColorIndex(percent);
  currentState = LEDState::MOISTURE_BLINKING;
  moistureBlinker.start(millis(), BLINK_DURATION_MS, MOISTURE_BLINK_COUNT);
  blinkColor = RGBColor(colorIndex);
  setColor(RGBColor(ColorIndex::OFF)); // Start with LED off
}

void RgbLedStatusDisplay::showMessage(const char* msg) {
  // LED can't display text - ignore
}

// Binary search for efficient moisture color lookup
ColorIndex RgbLedStatusDisplay::getMoistureColorIndex(float percent) {
  percent = constrain(percent, 0.0, 100.0);
  uint8_t intPercent = (uint8_t)percent;
  
  // Binary search through thresholds array
  uint8_t left = 0;
  uint8_t right = sizeof(MOISTURE_THRESHOLDS) / sizeof(MOISTURE_THRESHOLDS[0]);
  
  while (left < right) {
    uint8_t mid = (left + right) / 2;
    uint8_t threshold = pgm_read_byte(&MOISTURE_THRESHOLDS[mid]);
    
    if (intPercent <= threshold) {
      right = mid;
    } else {
      left = mid + 1;
    }
  }
  
  // Map search result to color index
  return static_cast<ColorIndex>(left + 1); // +1 because OFF is index 0
}

void RgbLedStatusDisplay::update() {
  // Safety check - skip if not initialized
  if (!isInitialized) {
    return;
  }
  
  uint32_t now = millis();
  
  // Single structured update with one return point
  bool continueProcessing = true;
  
  // Skip all state machine logic if in test mode
  if (currentState == LEDState::TEST_MODE) {
    continueProcessing = false;
  }
  
  // Handle boot timeout (replaces BOOT_GREEN_HOLD state)
  if (continueProcessing && currentState == LEDState::BOOT_GREEN && moistureBlinker.ready(now)) {
    if (!moistureBlinker.flip(now)) {
      // Boot timeout complete
      currentState = LEDState::OFF;
      setColor(RGBColor(ColorIndex::OFF));
      LOG_LED("Boot timeout complete - LED OFF");
    }
  }
  
  // Handle moisture blinks
  if (continueProcessing && currentState == LEDState::MOISTURE_BLINKING && moistureBlinker.ready(now)) {
    if (moistureBlinker.phase) {
      setColor(RGBColor(ColorIndex::OFF));
    } else {
      setColor(blinkColor);
    }
    
    if (!moistureBlinker.flip(now)) {
      // Blink sequence complete
      currentState = LEDState::OFF;
      setColor(RGBColor(ColorIndex::OFF));
      LOG_LED("Moisture blinks complete - LED OFF");
    }
  }
  
  // Handle connection failure blinks
  if (continueProcessing && currentState == LEDState::CONNECTION_FAILURE && failureBlinker.ready(now)) {
    if (failureBlinker.phase) {
      setColor(RGBColor(ColorIndex::OFF));
    } else {
      setColor(RGBColor(ColorIndex::RED));
    }
    failureBlinker.flip(now); // Infinite blink
  }
  
  // Handle commissioning states
  if (continueProcessing && commissioningBlinker.ready(now)) {
    switch (currentState) {
      case LEDState::COMMISSIONING_READY:
        // Fast white blink - check timeout
        if (now - stateStartTime > COMMISSIONING_TIMEOUT_MS) {
          LOG_LED("Commissioning timeout - returning to OFF");
          currentState = LEDState::OFF;
          setColor(RGBColor(ColorIndex::OFF));
        } else {
          // Continue blinking
          if (commissioningBlinker.phase) {
            setColor(RGBColor(ColorIndex::OFF));
          } else {
            setColor(RGBColor(ColorIndex::WHITE));
          }
          commissioningBlinker.flip(now);
        }
        break;
        
      case LEDState::COMMISSIONING_ACTIVE:
        // Slow green blink - check timeout
        if (now - stateStartTime > COMMISSIONING_TIMEOUT_MS) {
          LOG_LED("Commissioning timeout - failed");
          currentState = LEDState::COMMISSIONING_FAILED;
          commissioningBlinker.start(now, COMMISSIONING_FAST_BLINK_MS, 10);
          setColor(RGBColor(ColorIndex::RED));
        } else {
          // Continue blinking
          if (commissioningBlinker.phase) {
            setColor(RGBColor(ColorIndex::OFF));
          } else {
            setColor(RGBColor(ColorIndex::GREEN));
          }
          commissioningBlinker.flip(now);
        }
        break;
        
      case LEDState::COMMISSIONING_SUCCESS:
        // Solid green hold with timeout
        if (!commissioningBlinker.flip(now)) {
          LOG_LED("Commissioning success display complete - OFF");
          currentState = LEDState::OFF;
          setColor(RGBColor(ColorIndex::OFF));
        }
        break;
        
      case LEDState::COMMISSIONING_FAILED:
        // Fast red blinks with count limit
        if (commissioningBlinker.phase) {
          setColor(RGBColor(ColorIndex::OFF));
        } else {
          setColor(RGBColor(ColorIndex::RED));
        }
        
        if (!commissioningBlinker.flip(now)) {
          LOG_LED("Commissioning failed display complete - OFF");
          currentState = LEDState::OFF;
          setColor(RGBColor(ColorIndex::OFF));
        }
        break;
        
      default:
        // Not a commissioning state - continue
        break;
    }
  }
  
  // Handle battery flash (independent of state)
  if (continueProcessing && batteryBlinker.count > 0 && batteryBlinker.ready(now)) {
    if (batteryBlinker.phase) {
      // Flash OFF - restore previous state color
      if (currentState == LEDState::BOOT_GREEN) {
        setColor(RGBColor(ColorIndex::GREEN));
      } else if (currentState == LEDState::CONNECTION_FAILURE) {
        setColor(RGBColor(ColorIndex::RED));
      } else {
        setColor(RGBColor(ColorIndex::OFF));
      }
    } else {
      // Flash ON
      setColor(blinkColor);
    }
    
    if (!batteryBlinker.flip(now)) {
      // Flash complete
      batteryBlinker.count = 0; // Reset for next time
    }
  }
}

// Unified test method for debugging LED hardware
void RgbLedStatusDisplay::testColor(bool r, bool g, bool b) {
  LOG_LED("TEST MODE - ENTERING");
  // Force stop all ongoing sequences
  moistureBlinker.count = 0;
  failureBlinker.count = 0;
  batteryBlinker.count = 0;
  commissioningBlinker.count = 0;  // Stop commissioning blinks
  currentState = LEDState::TEST_MODE;  // Disable state machine completely
  setColor(r, g, b);
  LOG_LED("TEST COMMAND COMPLETED");
}
