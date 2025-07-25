#include "SensorManager.h"
#include <Arduino.h>

void SensorManager::begin() {
  pinMode(kMoisturePin, INPUT);
  calibrationManager.begin();
}

float SensorManager::readMoisture() {
  int raw = analogRead(kMoisturePin);
  int dryValue, wetValue;
  calibrationManager.getMoistureCalibration(dryValue, wetValue);
  
  float percent = constrain(map(raw, dryValue, wetValue, 0, 100), 0, 100);
  
  updateStatistics(percent);
  return percent;
}

void SensorManager::setCalibration(int dryValue, int wetValue) {
  calibrationManager.setMoistureCalibration(dryValue, wetValue);
  calibrationManager.saveCalibration();
}

void SensorManager::getCalibration(int& dryValue, int& wetValue) const {
  calibrationManager.getMoistureCalibration(dryValue, wetValue);
}

void SensorManager::startCalibration() {
  calibrationManager.startCalibration();
}

bool SensorManager::isCalibrating() const {
  return calibrationManager.isCalibrating();
}

void SensorManager::calibrateDry() {
  calibrationManager.calibrateDry();
}

void SensorManager::calibrateWet() {
  calibrationManager.calibrateWet();
}

void SensorManager::finishCalibration() {
  calibrationManager.finishCalibration();
}

void SensorManager::resetCalibration() {
  calibrationManager.resetToDefaults();
  calibrationManager.saveCalibration();
}

void SensorManager::resetStatistics() {
  minMoisture = 100.0;
  maxMoisture = 0.0;
}

void SensorManager::updateStatistics(float moisture) {
  minMoisture = min(minMoisture, moisture);
  maxMoisture = max(maxMoisture, moisture);
}
