#include "CalibrationManager.h"
#include <Arduino.h>
#include <EEPROM.h>

void CalibrationManager::begin() {
  // Silicon Labs EEPROM doesn't need begin() with size parameter
  loadCalibration();
}

void CalibrationManager::loadCalibration() {
  if (readFromEEPROM() && isCalibrationValid()) {
    dataLoaded = true;
  } else {
    // Invalid or missing data, use defaults
    resetToDefaults();
    saveCalibration(); // Save defaults to EEPROM
  }
}

void CalibrationManager::saveCalibration() {
  data.checksum = calculateChecksum(data);
  writeToEEPROM();
}

void CalibrationManager::resetToDefaults() {
  data.magicNumber = kEepromMagicNumber;
  data.version = kEepromVersion;
  data.moistureDry = kDefaultMoistureDry;
  data.moistureWet = kDefaultMoistureWet;
  data.batteryDivider = kDefaultBatteryDivider;
  dataLoaded = true;
}

void CalibrationManager::setMoistureCalibration(int dryValue, int wetValue) {
  data.moistureDry = dryValue;
  data.moistureWet = wetValue;
}

void CalibrationManager::getMoistureCalibration(int& dryValue, int& wetValue) const {
  dryValue = data.moistureDry;
  wetValue = data.moistureWet;
}

void CalibrationManager::setBatteryDivider(float divider) {
  data.batteryDivider = divider;
}

float CalibrationManager::getBatteryDivider() const {
  return data.batteryDivider;
}

void CalibrationManager::startCalibration() {
  calibrationMode = true;
  // Start with current values
  // User will call calibrateDry() and calibrateWet()
}

void CalibrationManager::calibrateDry() {
  if (calibrationMode) {
    // Read current ADC value as dry reference
    data.moistureDry = analogRead(kMoisturePin);
  }
}

void CalibrationManager::calibrateWet() {
  if (calibrationMode) {
    // Read current ADC value as wet reference
    data.moistureWet = analogRead(kMoisturePin);
  }
}

void CalibrationManager::finishCalibration() {
  if (calibrationMode) {
    calibrationMode = false;
    saveCalibration();
  }
}

bool CalibrationManager::isCalibrationValid() const {
  return (data.magicNumber == kEepromMagicNumber) &&
         (data.version == kEepromVersion) &&
         (data.moistureDry != data.moistureWet) &&
         (data.batteryDivider > 0.0) &&
         (calculateChecksum(data) == data.checksum);
}

uint8_t CalibrationManager::calculateChecksum(const CalibrationData& data) const {
  uint8_t checksum = 0;
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&data);
  size_t size = sizeof(CalibrationData) - sizeof(data.checksum); // Exclude checksum itself
  
  for (size_t i = 0; i < size; i++) {
    checksum ^= bytes[i];
  }
  return checksum;
}

void CalibrationManager::writeToEEPROM() {
  EEPROM.put(kEepromCalibrationAddress, data);
  // Silicon Labs EEPROM writes immediately, no commit() needed
}

bool CalibrationManager::readFromEEPROM() {
  EEPROM.get(kEepromCalibrationAddress, data);
  return true; // EEPROM.get always succeeds
}
