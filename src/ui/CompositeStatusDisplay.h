#pragma once
#include "StatusDisplay.h"

class CompositeStatusDisplay : public StatusDisplay {
public:
  CompositeStatusDisplay(StatusDisplay* primary, StatusDisplay* secondary = nullptr);
  ~CompositeStatusDisplay();
  
  void begin() override;
  void handleEvent(StatusEvent event) override;
  void showMoisture(float percent) override;
  void showMessage(const char* msg) override;
  void showBattery(float voltage, bool isLow = false) override;
  void update() override;
  
  // Management methods
  void setPrimary(StatusDisplay* display);
  void setSecondary(StatusDisplay* display);
  StatusDisplay* getPrimary() const { return primaryDisplay; }
  StatusDisplay* getSecondary() const { return secondaryDisplay; }

private:
  StatusDisplay* primaryDisplay;
  StatusDisplay* secondaryDisplay;
  bool ownsDisplays; // Whether to delete displays in destructor
};
