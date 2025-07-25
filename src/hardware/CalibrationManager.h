#pragma once
#include "../config/Config.h"

struct CalibrationData {
  uint16_t magicNumber;     // Validation magic number
  uint8_t version;          // Data structure version
  int moistureDry;          // ADC value for dry soil
  int moistureWet;          // ADC value for wet soil
  float batteryDivider;     // Battery voltage divider ratio
  uint8_t checksum;         // Simple checksum for data integrity
};

class CalibrationManager {
public:
  void begin();
  
  // Calibration data access
  void loadCalibration();
  void saveCalibration();
  void resetToDefaults();
  
  // Moisture sensor calibration
  void setMoistureCalibration(int dryValue, int wetValue);
  void getMoistureCalibration(int& dryValue, int& wetValue) const;
  
  // Battery calibration
  void setBatteryDivider(float divider);
  float getBatteryDivider() const;
  
  // Calibration process helpers
  void startCalibration();
  bool isCalibrating() const { return calibrationMode; }
  void calibrateDry();
  void calibrateWet();
  void finishCalibration();
  
  // Data validation
  bool isCalibrationValid() const;
  
private:
  CalibrationData data;
  bool calibrationMode = false;
  bool dataLoaded = false;
  
  uint8_t calculateChecksum(const CalibrationData& data) const;
  void writeToEEPROM();
  bool readFromEEPROM();
};
