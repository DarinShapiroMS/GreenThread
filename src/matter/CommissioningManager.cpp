#include "CommissioningManager.h"

// ButtonCommissioning implementation
void ButtonCommissioning::begin() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Built-in pull-up
  lastButtonState = digitalRead(BUTTON_PIN);
  state = CommissioningState::IDLE;
  
  Serial.println(F("[Commissioning] Button initialized - long press to commission"));
}

void ButtonCommissioning::update() {
  updateButtonState();
  
  uint32_t now = millis();
  
  // Handle commissioning timeout
  if (state == CommissioningState::READY || state == CommissioningState::IN_PROGRESS) {
    if (now - commissioningStartTime > 180000) {  // 3 minutes timeout
      Serial.println(F("[Commissioning] Timeout - returning to idle"));
      transitionToState(CommissioningState::FAILED);
      // Failed state will auto-transition to IDLE after LED sequence
    }
  }
  
  // Auto-transition from terminal states
  if (state == CommissioningState::SUCCESS || state == CommissioningState::FAILED) {
    // Wait for LED display to complete its sequence (handled by LED timing)
    // Then transition back to idle - this is managed by the LED state machine
    if (now - commissioningStartTime > 5000) {  // 5 seconds total display time
      transitionToState(CommissioningState::IDLE);
    }
  }
}

void ButtonCommissioning::updateButtonState() {
  bool currentReading = digitalRead(BUTTON_PIN);
  uint32_t now = millis();
  
  // Debounce logic
  if (currentReading != lastButtonState) {
    lastDebounceTime = now;
  }
  
  if ((now - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
    // Stable reading
    if (currentReading != buttonPressed) {
      buttonPressed = currentReading;
      
      if (buttonPressed == LOW) {  // Button pressed (active LOW)
        buttonPressStart = now;
        Serial.println(F("[Commissioning] Button pressed"));
      } else {  // Button released
        uint32_t pressDuration = now - buttonPressStart;
        
        if (pressDuration >= BUTTON_FACTORY_RESET_MS) {
          handleFactoryReset();
        } else if (pressDuration >= BUTTON_LONG_PRESS_MS) {
          handleLongPress();
        } else {
          handleButtonPress();
        }
      }
    }
  }
  
  lastButtonState = currentReading;
}

void ButtonCommissioning::handleButtonPress() {
  Serial.println(F("[Commissioning] Short press - ignored"));
  // Short press does nothing - prevents accidental commissioning
}

void ButtonCommissioning::handleLongPress() {
  Serial.println(F("[Commissioning] Long press - starting commissioning mode"));
  
  if (state == CommissioningState::IDLE) {
    startCommissioning();
  } else {
    Serial.println(F("[Commissioning] Already active - ignoring"));
  }
}

void ButtonCommissioning::handleFactoryReset() {
  Serial.println(F("[Commissioning] Factory reset initiated"));
  
  transitionToState(CommissioningState::FACTORY_RESET);
  
  // TODO: Add actual Matter factory reset here
  // For now, just signal the event
  if (statusDisplay) {
    statusDisplay->handleEvent(StatusEvent::FactoryReset);
  }
  
  // Return to idle after reset
  transitionToState(CommissioningState::IDLE);
}

void ButtonCommissioning::startCommissioning() {
  if (state != CommissioningState::IDLE) {
    Serial.println(F("[Commissioning] Cannot start - not idle"));
    return;
  }
  
  transitionToState(CommissioningState::READY);
  commissioningStartTime = millis();
  
  // TODO: Add actual Matter commissioning start here
  // For now, simulate progression after 2 seconds
  Serial.println(F("[Commissioning] Commissioning mode active"));
  Serial.println(F("[Commissioning] TODO: Start Matter commissioning advertisements"));
}

void ButtonCommissioning::stopCommissioning() {
  if (state == CommissioningState::IDLE) {
    return;
  }
  
  Serial.println(F("[Commissioning] Stopping commissioning"));
  transitionToState(CommissioningState::IDLE);
  
  // TODO: Stop Matter commissioning advertisements
}

void ButtonCommissioning::transitionToState(CommissioningState newState) {
  if (state == newState) {
    return;
  }
  
  Serial.print(F("[Commissioning] State transition: "));
  Serial.print((int)state);
  Serial.print(F(" -> "));
  Serial.println((int)newState);
  
  state = newState;
  
  // Notify status display of state changes
  if (statusDisplay) {
    switch (newState) {
      case CommissioningState::IDLE:
        // No specific event - just return to normal operation
        break;
        
      case CommissioningState::READY:
        statusDisplay->handleEvent(StatusEvent::CommissioningModeActive);
        break;
        
      case CommissioningState::IN_PROGRESS:
        statusDisplay->handleEvent(StatusEvent::CommissioningInProgress);
        break;
        
      case CommissioningState::SUCCESS:
        statusDisplay->handleEvent(StatusEvent::CommissioningSuccess);
        break;
        
      case CommissioningState::FAILED:
        statusDisplay->handleEvent(StatusEvent::CommissioningFailed);
        break;
        
      case CommissioningState::FACTORY_RESET:
        statusDisplay->handleEvent(StatusEvent::FactoryReset);
        break;
    }
  }
}

// CommissioningManager implementation
void CommissioningManager::begin() {
  Serial.println(F("[Commissioning] Manager starting..."));
  currentMethod->begin();
  Serial.println(F("[Commissioning] Manager ready"));
}

void CommissioningManager::update() {
  currentMethod->update();
  
  // Future: Handle method switching, global commissioning logic
}
