#pragma once

enum class StatusEvent {
  BootStarting,
  BootSensorInit,
  BootNetworkInit,
  BootMatterInit,
  BootComplete,
  ThreadConnected,
  ThreadDisconnected,
  ThreadConnectionFailed,
  MatterOnline,
  MatterOffline,
  MatterConnectionFailed,
  // Commissioning events - extensible for future methods
  CommissioningButtonPressed,
  CommissioningModeActive,
  CommissioningInProgress,
  CommissioningSuccess,
  CommissioningFailed,
  CommissioningTimeout,
  FactoryReset,
  MoisturePublishing,
  MoisturePublished,
  EnteringSleep,
  BatteryLow,
  Error,
};

class StatusDisplay {
public:
  virtual void begin() = 0;
  virtual void handleEvent(StatusEvent event) = 0;
  virtual void showMoisture(float percent) = 0;
  virtual void showMessage(const char* msg) = 0;
  virtual void showBattery(float voltage, bool isLow = false) {};  // Optional battery display
  virtual void update() = 0;
  
  // Test methods for debugging (optional implementation)
  virtual void testRed() {};    // Test red color/output
  virtual void testGreen() {};  // Test green color/output  
  virtual void testBlue() {};   // Test blue color/output
  virtual void testOff() {};    // Test turning off
};
