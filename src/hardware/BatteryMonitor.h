#pragma once
#include "../config/Config.h"
#include "CalibrationManager.h"

enum class BatteryStatus {
  Normal,
  Low,
  Critical
};

class BatteryMonitor {
public:
  void begin();
  void setCalibrationManager(CalibrationManager* manager) { calibrationManager = manager; }
  
  float readVoltage();
  BatteryStatus getStatus();
  bool isLow() const;
  
  // Calibration methods
  void setVoltageDivider(float ratio);
  float getVoltageDivider() const;
  
  // Threshold management
  void setLowThreshold(float threshold);
  float getLowThreshold() const { return lowThreshold; }

private:
  CalibrationManager* calibrationManager = nullptr;
  float lowThreshold = kBatteryLowThresh;
  float lastVoltage = 0.0;
};
