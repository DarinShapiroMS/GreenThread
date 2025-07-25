#include "PowerManager.h"
#include <Arduino.h>

void PowerManager::begin() {
  loadDefaultConfiguration();
  currentState = PowerState::Booting;
  lastState = PowerState::Booting;
  stateChangeTime = millis();
  totalSleepTime = 0;
  sleepCycles = 0;
}

void PowerManager::updatePowerState(float batteryVoltage, bool usbConnected) {
  PowerState newState = currentState;
  
  if (usbConnected && config.usbOverridePowerManagement) {
    newState = PowerState::UsbPowered;
  } else if (batteryVoltage < config.batteryShutdownThresh) {
    newState = PowerState::Critical;
  } else if (batteryVoltage < config.batteryCriticalThresh) {
    newState = PowerState::LowPower;
  } else if (batteryVoltage < config.batteryExtendedThresh) {
    newState = PowerState::Extended;
  } else if (batteryVoltage >= config.batteryNormalThresh) {
    newState = PowerState::Normal;
  }
  
  if (newState != currentState) {
    lastState = currentState;
    currentState = newState;
    stateChangeTime = millis();
  }
}

uint32_t PowerManager::getCurrentSleepInterval() const {
  if (!config.enablePowerManagement) {
    return 0; // No sleep when power management disabled
  }
  
  switch (currentState) {
    case PowerState::UsbPowered:
      return config.usbSleepInterval;
    case PowerState::Normal:
      return config.normalSleepInterval;
    case PowerState::Extended:
      return config.extendedSleepInterval;
    case PowerState::LowPower:
      return config.lowPowerSleepInterval;
    case PowerState::Critical:
      return config.maxSleepInterval; // Maximum conservation
    case PowerState::Booting:
    default:
      return config.normalSleepInterval;
  }
}

void PowerManager::setConfiguration(const PowerConfiguration& newConfig) {
  config = newConfig;
  validateConfiguration();
}

void PowerManager::setNormalSleepInterval(uint32_t interval) {
  config.normalSleepInterval = constrainSleepInterval(interval);
}

void PowerManager::setExtendedSleepInterval(uint32_t interval) {
  config.extendedSleepInterval = constrainSleepInterval(interval);
}

void PowerManager::setLowPowerSleepInterval(uint32_t interval) {
  config.lowPowerSleepInterval = constrainSleepInterval(interval);
}

void PowerManager::setUsbSleepInterval(uint32_t interval) {
  config.usbSleepInterval = constrainSleepInterval(interval);
}

void PowerManager::setBatteryNormalThresh(float thresh) {
  config.batteryNormalThresh = constrain(thresh, 2.5, 4.5);
}

void PowerManager::setBatteryExtendedThresh(float thresh) {
  config.batteryExtendedThresh = constrain(thresh, 2.5, 4.5);
}

void PowerManager::setBatteryCriticalThresh(float thresh) {
  config.batteryCriticalThresh = constrain(thresh, 2.5, 4.5);
}

void PowerManager::setBatteryShutdownThresh(float thresh) {
  config.batteryShutdownThresh = constrain(thresh, 2.0, 4.0);
}

bool PowerManager::shouldEnterSleep() const {
  return config.enablePowerManagement && (currentState != PowerState::Booting);
}

bool PowerManager::shouldWakeUp() const {
  // Add logic for remote wakeup, button press, etc.
  return config.allowRemoteWakeup;
}

void PowerManager::enterSleepMode() {
  if (shouldEnterSleep()) {
    sleepCycles++;
    // Platform-specific sleep implementation would go here
  }
}

void PowerManager::wakeFromSleep() {
  uint32_t sleepDuration = getCurrentSleepInterval();
  totalSleepTime += sleepDuration;
}

void PowerManager::loadDefaultConfiguration() {
  config.normalSleepInterval = kNormalSleepInterval;
  config.extendedSleepInterval = kExtendedSleepInterval;
  config.lowPowerSleepInterval = kLowPowerSleepInterval;
  config.usbSleepInterval = kUsbSleepInterval;
  
  config.batteryNormalThresh = kBatteryNormalThresh;
  config.batteryExtendedThresh = kBatteryExtendedThresh;
  config.batteryCriticalThresh = kBatteryCriticalThresh;
  config.batteryShutdownThresh = kBatteryShutdownThresh;
  
  config.maxSleepInterval = kMaxSleepInterval;
  config.minSleepInterval = kMinSleepInterval;
  config.allowRemoteWakeup = kAllowRemoteWakeup;
  config.usbOverridePowerManagement = kUsbOverridePowerManagement;
  config.enablePowerManagement = kEnablePowerManagement;
}

void PowerManager::validateConfiguration() {
  // Ensure thresholds are in logical order
  if (config.batteryShutdownThresh >= config.batteryCriticalThresh) {
    config.batteryShutdownThresh = config.batteryCriticalThresh - 0.1;
  }
  if (config.batteryCriticalThresh >= config.batteryExtendedThresh) {
    config.batteryCriticalThresh = config.batteryExtendedThresh - 0.1;
  }
  if (config.batteryExtendedThresh >= config.batteryNormalThresh) {
    config.batteryExtendedThresh = config.batteryNormalThresh - 0.1;
  }
  
  // Constrain all sleep intervals
  config.normalSleepInterval = constrainSleepInterval(config.normalSleepInterval);
  config.extendedSleepInterval = constrainSleepInterval(config.extendedSleepInterval);
  config.lowPowerSleepInterval = constrainSleepInterval(config.lowPowerSleepInterval);
  config.usbSleepInterval = constrainSleepInterval(config.usbSleepInterval);
}

uint32_t PowerManager::constrainSleepInterval(uint32_t interval) const {
  return constrain(interval, config.minSleepInterval, config.maxSleepInterval);
}
