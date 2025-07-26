#pragma once
#include "../config/Config.h"
#include "CalibrationManager.h"

enum class BatteryStatus {
  Normal,
  Low,
  Critical,
  NotConnected,  // No battery physically connected
  Dead           // Battery connected but completely discharged
};

enum class BatteryState {
  Healthy,       // Battery connected and functional
  NotPresent,    // No battery detected (floating pin)
  DeadBattery,   // Battery present but dead/critically low
  Unknown        // Unable to determine state
};

class BatteryMonitor {
public:
  void begin();
  void setCalibrationManager(CalibrationManager* manager) { calibrationManager = manager; }
  
  float readVoltage();
  BatteryStatus getStatus();
  BatteryState getBatteryState();
  bool isLow() const;
  bool isBatteryConnected() const;
  bool isBatteryDead() const;
  const char* getBatteryStatusString() const;
  const char* getBatteryStateString() const;
  
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
