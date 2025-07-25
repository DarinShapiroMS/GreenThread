#pragma once
#include "StatusDisplay.h"

class RgbLedStatusDisplay : public StatusDisplay {
public:
  void begin() override;
  void handleEvent(StatusEvent event) override;
  void showMoisture(float percent) override;
  void showMessage(const char* msg) override;
  void update() override;

private:
  void setColor(bool r, bool g, bool b);
};
