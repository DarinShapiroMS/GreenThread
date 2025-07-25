#pragma once

enum class StatusEvent {
  Booting,
  ThreadConnected,
  ThreadDisconnected,
  MatterOnline,
  MatterOffline,
  MoisturePublished,
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
};
