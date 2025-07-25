#pragma once
#include "../config/Config.h"

enum class PowerState {
  Booting,
  Normal,       // Normal operation, standard intervals
  Extended,     // Extended intervals for battery conservation  
  LowPower,     // Emergency power conservation
  Critical,     // Protective shutdown imminent
  UsbPowered    // Connected to USB, more responsive
};

struct PowerConfiguration {
  // Sleep intervals (milliseconds)
  uint32_t normalSleepInterval;
  uint32_t extendedSleepInterval; 
  uint32_t lowPowerSleepInterval;
  uint32_t usbSleepInterval;
  
  // Battery thresholds (volts)
  float batteryNormalThresh;
  float batteryExtendedThresh;
  float batteryCriticalThresh;
  float batteryShutdownThresh;
  
  // Limits and behavior
  uint32_t maxSleepInterval;
  uint32_t minSleepInterval;
  bool allowRemoteWakeup;
  bool usbOverridePowerManagement;
  bool enablePowerManagement;
};

class PowerManager {
public:
  void begin();
  
  // State management
  PowerState getCurrentState() const { return currentState; }
  void updatePowerState(float batteryVoltage, bool usbConnected);
  uint32_t getCurrentSleepInterval() const;
  
  // Configuration management (Matter attribute interface)
  PowerConfiguration getConfiguration() const { return config; }
  void setConfiguration(const PowerConfiguration& newConfig);
  
  // Individual setting methods (for Matter attribute mapping)
  void setNormalSleepInterval(uint32_t interval);
  void setExtendedSleepInterval(uint32_t interval);  
  void setLowPowerSleepInterval(uint32_t interval);
  void setUsbSleepInterval(uint32_t interval);
  
  void setBatteryNormalThresh(float thresh);
  void setBatteryExtendedThresh(float thresh);
  void setBatteryCriticalThresh(float thresh);
  void setBatteryShutdownThresh(float thresh);
  
  uint32_t getNormalSleepInterval() const { return config.normalSleepInterval; }
  uint32_t getExtendedSleepInterval() const { return config.extendedSleepInterval; }
  uint32_t getLowPowerSleepInterval() const { return config.lowPowerSleepInterval; }
  uint32_t getUsbSleepInterval() const { return config.usbSleepInterval; }
  
  float getBatteryNormalThresh() const { return config.batteryNormalThresh; }
  float getBatteryExtendedThresh() const { return config.batteryExtendedThresh; }
  float getBatteryCriticalThresh() const { return config.batteryCriticalThresh; }
  float getBatteryShutdownThresh() const { return config.batteryShutdownThresh; }
  
  // Power management actions
  bool shouldEnterSleep() const;
  bool shouldWakeUp() const;
  void enterSleepMode();
  void wakeFromSleep();
  
  // Statistics and diagnostics
  uint32_t getTotalSleepTime() const { return totalSleepTime; }
  uint32_t getSleepCycles() const { return sleepCycles; }
  PowerState getLastState() const { return lastState; }
  
private:
  PowerConfiguration config;
  PowerState currentState;
  PowerState lastState;
  uint32_t stateChangeTime;
  uint32_t totalSleepTime;
  uint32_t sleepCycles;
  
  void loadDefaultConfiguration();
  void validateConfiguration();
  uint32_t constrainSleepInterval(uint32_t interval) const;
};
