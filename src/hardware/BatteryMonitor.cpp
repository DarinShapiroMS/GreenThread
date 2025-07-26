#include "BatteryMonitor.h"
#include <Arduino.h>

void BatteryMonitor::begin() {
  pinMode(kBatteryPin, INPUT);
}

float BatteryMonitor::readVoltage() {
  int raw = analogRead(kBatteryPin);
  
  float voltageDivider = kBatteryVoltageDivider; // Default
  if (calibrationManager) {
    voltageDivider = calibrationManager->getBatteryDivider();
  }
  
  float voltage = (raw / kAdcReference) * voltageDivider;
  
  // Enhanced battery detection logic
  // Check multiple readings for stability
  static float lastReadings[3] = {0, 0, 0};
  static int readingIndex = 0;
  
  lastReadings[readingIndex] = voltage;
  readingIndex = (readingIndex + 1) % 3;
  
  // Calculate average of last 3 readings
  float avgVoltage = (lastReadings[0] + lastReadings[1] + lastReadings[2]) / 3.0;
  
  // Determine if battery is present based on voltage characteristics
  if (avgVoltage < 0.5) {
    // Very low voltage - likely floating pin (no battery)
    lastVoltage = -1.0;  // Indicate no battery
  } else if (avgVoltage > 5.5) {
    // Very high voltage - likely floating pin or error
    lastVoltage = -1.0;  // Indicate no battery
  } else if (avgVoltage < 2.0) {
    // Low but measurable voltage - dead battery
    lastVoltage = avgVoltage;  // Keep the low reading
  } else {
    // Normal battery voltage range
    lastVoltage = avgVoltage;
  }
  
  return lastVoltage;
}

BatteryStatus BatteryMonitor::getStatus() {
  float voltage = readVoltage();
  
  // Determine battery status based on voltage
  if (voltage < 0) {
    // No battery detected (floating pin)
    return BatteryStatus::NotConnected;
  } else if (voltage < 2.0) {
    // Battery present but dead/critically low
    return BatteryStatus::Dead;
  } else if (voltage < lowThreshold * 0.85) {
    // Critical battery level
    return BatteryStatus::Critical;
  } else if (voltage < lowThreshold) {
    // Low battery level
    return BatteryStatus::Low;
  } else {
    // Normal battery level
    return BatteryStatus::Normal;
  }
}

bool BatteryMonitor::isLow() const {
  return lastVoltage > 0 && lastVoltage < lowThreshold;
}

bool BatteryMonitor::isBatteryConnected() const {
  BatteryStatus status = const_cast<BatteryMonitor*>(this)->getStatus();
  return status != BatteryStatus::NotConnected;
}

bool BatteryMonitor::isBatteryDead() const {
  BatteryStatus status = const_cast<BatteryMonitor*>(this)->getStatus();
  return status == BatteryStatus::Dead;
}

void BatteryMonitor::setVoltageDivider(float ratio) {
  if (calibrationManager) {
    calibrationManager->setBatteryDivider(ratio);
    calibrationManager->saveCalibration();
  }
}

float BatteryMonitor::getVoltageDivider() const {
  if (calibrationManager) {
    return calibrationManager->getBatteryDivider();
  }
  return kBatteryVoltageDivider;
}

void BatteryMonitor::setLowThreshold(float threshold) {
  lowThreshold = threshold;
}

BatteryState BatteryMonitor::getBatteryState() {
  BatteryStatus status = getStatus();
  
  switch (status) {
    case BatteryStatus::NotConnected:
      return BatteryState::NotPresent;
    case BatteryStatus::Dead:
      return BatteryState::DeadBattery;
    case BatteryStatus::Normal:
    case BatteryStatus::Low:
    case BatteryStatus::Critical:
      return BatteryState::Healthy;
    default:
      return BatteryState::Unknown;
  }
}

const char* BatteryMonitor::getBatteryStatusString() const {
  BatteryStatus status = const_cast<BatteryMonitor*>(this)->getStatus();
  switch (status) {
    case BatteryStatus::Normal:    return PSTR("Normal");
    case BatteryStatus::Low:       return PSTR("Low"); 
    case BatteryStatus::Critical:  return PSTR("Critical");
    case BatteryStatus::NotConnected: return PSTR("Not Connected");
    case BatteryStatus::Dead:      return PSTR("Dead");
    default:                       return PSTR("Unknown");
  }
}

const char* BatteryMonitor::getBatteryStateString() const {
  BatteryState state = const_cast<BatteryMonitor*>(this)->getBatteryState();
  switch (state) {
    case BatteryState::Healthy:     return PSTR("Healthy");
    case BatteryState::NotPresent:  return PSTR("No Battery");
    case BatteryState::DeadBattery: return PSTR("Dead Battery");
    case BatteryState::Unknown:     return PSTR("Unknown");
    default:                        return PSTR("Error");
  }
}
