#include "CompositeStatusDisplay.h"

CompositeStatusDisplay::CompositeStatusDisplay(StatusDisplay* primary, StatusDisplay* secondary)
  : primaryDisplay(primary), secondaryDisplay(secondary), ownsDisplays(true) {
}

CompositeStatusDisplay::~CompositeStatusDisplay() {
  if (ownsDisplays) {
    delete primaryDisplay;
    delete secondaryDisplay;
  }
}

void CompositeStatusDisplay::begin() {
  if (primaryDisplay) {
    primaryDisplay->begin();
  }
  if (secondaryDisplay) {
    secondaryDisplay->begin();
  }
}

void CompositeStatusDisplay::handleEvent(StatusEvent event) {
  if (primaryDisplay) {
    primaryDisplay->handleEvent(event);
  }
  if (secondaryDisplay) {
    secondaryDisplay->handleEvent(event);
  }
}

void CompositeStatusDisplay::showMoisture(float percent) {
  if (primaryDisplay) {
    primaryDisplay->showMoisture(percent);
  }
  if (secondaryDisplay) {
    secondaryDisplay->showMoisture(percent);
  }
}

void CompositeStatusDisplay::showMessage(const char* msg) {
  if (primaryDisplay) {
    primaryDisplay->showMessage(msg);
  }
  if (secondaryDisplay) {
    secondaryDisplay->showMessage(msg);
  }
}

void CompositeStatusDisplay::showBattery(float voltage, bool isLow) {
  if (primaryDisplay) {
    primaryDisplay->showBattery(voltage, isLow);
  }
  if (secondaryDisplay) {
    secondaryDisplay->showBattery(voltage, isLow);
  }
}

void CompositeStatusDisplay::update() {
  if (primaryDisplay) {
    primaryDisplay->update();
  }
  if (secondaryDisplay) {
    secondaryDisplay->update();
  }
}

void CompositeStatusDisplay::setPrimary(StatusDisplay* display) {
  if (ownsDisplays && primaryDisplay) {
    delete primaryDisplay;
  }
  primaryDisplay = display;
}

void CompositeStatusDisplay::setSecondary(StatusDisplay* display) {
  if (ownsDisplays && secondaryDisplay) {
    delete secondaryDisplay;
  }
  secondaryDisplay = display;
}
