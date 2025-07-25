#pragma once
#include "../config/Config.h"
#include "StatusDisplay.h"

class DisplayFactory {
public:
  enum class DisplayType {
    Auto,
    OLED,
    RGB_LED,
    Serial
  };
  
  // Main factory method
  static StatusDisplay* createPrimaryDisplay();
  static StatusDisplay* createSecondaryDisplay(); // For USB serial support
  
  // Detection methods
  static bool isOledAvailable();
  static bool isUsbConnected();
  static DisplayType detectBestDisplay();
  
  // Display creation methods
  static StatusDisplay* createOledDisplay();
  static StatusDisplay* createRgbLedDisplay();
  static StatusDisplay* createSerialDisplay();
  
private:
  static void initializeI2cForDetection();
  static void cleanupI2cAfterDetection();
};
