// OledStatusDisplay.h

#pragma once
#include "StatusDisplay.h"

class OledStatusDisplay : public StatusDisplay {
public:
  void begin() override;
  void handleEvent(StatusEvent event) override;
  void showMoisture(float percent) override;
  void showMessage(const char* msg) override;
  void showBattery(float voltage, bool isLow = false) override;
  void update() override;

private:
  void drawCentered(const char* msg);
  void drawBatteryIcon(int x, int y, float voltage, bool isLow);
};
