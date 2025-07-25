#pragma once
#include "../config/Config.h"
#include "CalibrationManager.h"

class SensorManager {
public:
  void begin();
  float readMoisture();
  
  // Calibration methods
  void setCalibration(int dryValue, int wetValue);
  void getCalibration(int& dryValue, int& wetValue) const;
  
  // Calibration process
  void startCalibration();
  bool isCalibrating() const;
  void calibrateDry();
  void calibrateWet();
  void finishCalibration();
  void resetCalibration();
  
  // Statistics (optional for future use)
  float getMinMoisture() const { return minMoisture; }
  float getMaxMoisture() const { return maxMoisture; }
  void resetStatistics();

private:
  CalibrationManager calibrationManager;
  float minMoisture = 100.0;
  float maxMoisture = 0.0;
  
  void updateStatistics(float moisture);
};
