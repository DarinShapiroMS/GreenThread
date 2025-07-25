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
  
  lastVoltage = (raw / kAdcReference) * voltageDivider;
  return lastVoltage;
}

BatteryStatus BatteryMonitor::getStatus() {
  float voltage = readVoltage();
  
  if (voltage < lowThreshold * 0.85) {  // Critical at 85% of low threshold
    return BatteryStatus::Critical;
  } else if (voltage < lowThreshold) {
    return BatteryStatus::Low;
  } else {
    return BatteryStatus::Normal;
  }
}

bool BatteryMonitor::isLow() const {
  return lastVoltage < lowThreshold;
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
