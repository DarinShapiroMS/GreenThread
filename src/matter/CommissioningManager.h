#pragma once
#include <Arduino.h>
#include "../ui/StatusDisplay.h"

// Arduino Nano Matter built-in button pin
constexpr uint8_t BUTTON_PIN = 7;  // Built-in button on Arduino Nano Matter

// Button timing constants
constexpr uint16_t BUTTON_DEBOUNCE_MS = 50;        // Debounce time
constexpr uint16_t BUTTON_LONG_PRESS_MS = 3000;    // Long press for commissioning
constexpr uint16_t BUTTON_FACTORY_RESET_MS = 10000; // Very long press for factory reset

// Commissioning state management
enum class CommissioningState : uint8_t {
  IDLE,              // Normal operation
  READY,             // Ready to commission (after button press)
  IN_PROGRESS,       // Matter commissioning active
  SUCCESS,           // Commissioning completed successfully
  FAILED,            // Commissioning failed
  FACTORY_RESET      // Factory reset initiated
};

// Abstract commissioning method interface for future extensibility
class CommissioningMethod {
public:
  virtual void begin() = 0;
  virtual void update() = 0;
  virtual bool isActive() const = 0;
  virtual void startCommissioning() = 0;
  virtual void stopCommissioning() = 0;
  virtual CommissioningState getState() const = 0;
  virtual ~CommissioningMethod() = default;
};

// Button-based commissioning implementation
class ButtonCommissioning : public CommissioningMethod {
public:
  ButtonCommissioning(StatusDisplay* display) : statusDisplay(display) {}
  
  void begin() override;
  void update() override;
  bool isActive() const override { return state != CommissioningState::IDLE; }
  void startCommissioning() override;
  void stopCommissioning() override;
  CommissioningState getState() const override { return state; }
  
  // Button-specific methods
  void handleButtonPress();
  void handleLongPress();
  void handleFactoryReset();

private:
  StatusDisplay* statusDisplay;
  CommissioningState state = CommissioningState::IDLE;
  
  // Button state tracking
  bool buttonPressed = false;
  bool lastButtonState = HIGH;  // Pull-up, so HIGH = not pressed
  uint32_t buttonPressStart = 0;
  uint32_t lastDebounceTime = 0;
  
  // Commissioning timing
  uint32_t commissioningStartTime = 0;
  
  void updateButtonState();
  void transitionToState(CommissioningState newState);
};

// Main commissioning manager - supports multiple methods
class CommissioningManager {
public:
  CommissioningManager(StatusDisplay* display) : 
    buttonMethod(display), 
    statusDisplay(display),
    currentMethod(&buttonMethod) {}
  
  void begin();
  void update();
  
  // Commissioning control
  void startCommissioning() { currentMethod->startCommissioning(); }
  void stopCommissioning() { currentMethod->stopCommissioning(); }
  bool isActive() const { return currentMethod->isActive(); }
  CommissioningState getState() const { return currentMethod->getState(); }
  
  // Future: Add methods for QR code, NFC, etc.
  // void setCommissioningMethod(CommissioningMethod* method) { currentMethod = method; }

private:
  ButtonCommissioning buttonMethod;
  StatusDisplay* statusDisplay;
  CommissioningMethod* currentMethod;  // Extensible for future methods
};
