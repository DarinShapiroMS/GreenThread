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
  // Always add serial as secondary display when USB is connected
  if (isUsbConnected() && kEnableSerialWhenUsbConnected) {
    Serial.println("[Display] Adding Serial as secondary display (USB connected)");
    return createSerialDisplay();
  }
  return nullptr; // No secondary display needed
}

DisplayFactory::DisplayType DisplayFactory::detectBestDisplay() {
  // TEMPORARY: Force RGB LED for testing - comment out OLED detection
  /*
  if (isOledAvailable()) {
    Serial.println("[Display] Selected: OLED");
    return DisplayType::OLED;
  } else {
  */
    Serial.println("[Display] FORCED: RGB LED (for testing)");
    return DisplayType::RGB_LED; // Force RGB LED for testing
  //}
}

bool DisplayFactory::isOledAvailable() {
  initializeI2cForDetection();
  
  Serial.print("[Display] Checking for OLED at 0x");
  Serial.print(kOledI2cAddress, HEX);
  Serial.print("... ");
  
  Wire.beginTransmission(kOledI2cAddress);
  uint8_t error = Wire.endTransmission();
  bool available = (error == 0);
  
  if (available) {
    Serial.println("FOUND!");
  } else {
    Serial.print("NOT FOUND (error ");
    Serial.print(error);
    Serial.println(")");
  }
  
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
