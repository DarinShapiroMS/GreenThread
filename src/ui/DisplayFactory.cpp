#include "DisplayFactory.h"
#include "OledStatusDisplay.h"
#include "RgbLedStatusDisplay.h"
#include "SerialStatusDisplay.h"
#include <Wire.h>

StatusDisplay* DisplayFactory::createPrimaryDisplay() {
  DisplayType bestType = detectBestDisplay();
  
  switch (bestType) {
    case DisplayType::OLED:
      return createOledDisplay();
    case DisplayType::RGB_LED:
      return createRgbLedDisplay();
    case DisplayType::Serial:
    default:
      return createSerialDisplay();
  }
}

StatusDisplay* DisplayFactory::createSecondaryDisplay() {
  // Only create secondary display if USB is connected and it's not the primary
  if (isUsbConnected()) {
    DisplayType primaryType = detectBestDisplay();
    if (primaryType != DisplayType::Serial) {
      return createSerialDisplay(); // Add serial as secondary when on USB
    }
  }
  return nullptr; // No secondary display needed
}

DisplayFactory::DisplayType DisplayFactory::detectBestDisplay() {
  // Priority order: OLED > RGB_LED > Serial
  if (isOledAvailable()) {
    return DisplayType::OLED;
  } else if (isUsbConnected() && kEnableSerialWhenUsbConnected) {
    return DisplayType::Serial; // Prefer Serial when on USB and no OLED
  } else {
    return DisplayType::RGB_LED; // Default to LED when battery powered
  }
}

bool DisplayFactory::isOledAvailable() {
  initializeI2cForDetection();
  
  Wire.beginTransmission(kOledI2cAddress);
  bool available = (Wire.endTransmission() == 0);
  
  cleanupI2cAfterDetection();
  return available;
}

bool DisplayFactory::isUsbConnected() {
  // Simple heuristic: if Serial is available and responds quickly
  // This is a basic check - more sophisticated detection could be added
  unsigned long startTime = millis();
  Serial.begin(kSerialBaudRate);
  delay(10); // Brief delay for serial to initialize
  
  // Check if serial is ready (indicates USB connection)
  bool usbConnected = Serial;
  
  return usbConnected;
}

StatusDisplay* DisplayFactory::createOledDisplay() {
  return new OledStatusDisplay();
}

StatusDisplay* DisplayFactory::createRgbLedDisplay() {
  return new RgbLedStatusDisplay();
}

StatusDisplay* DisplayFactory::createSerialDisplay() {
  return new SerialStatusDisplay();
}

void DisplayFactory::initializeI2cForDetection() {
  Wire.begin();
  // Brief delay to ensure I2C is ready
  delay(10);
}

void DisplayFactory::cleanupI2cAfterDetection() {
  // I2C stays initialized for potential OLED use
  // Wire.end(); // Don't end if we might use OLED
}
