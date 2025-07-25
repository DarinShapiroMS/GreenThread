//#include <Arduino.h>
#include <Wire.h>
#include "src/config/Config.h"
#include "src/matter/MatterMultiSensor.h"
#include "src/hardware/SensorManager.h"
#include "src/hardware/BatteryMonitor.h"
#include "src/hardware/CalibrationManager.h"
#include "src/hardware/PowerManager.h"

#include "src/ui/StatusDisplay.h"
#include "src/ui/DisplayFactory.h"
#include "src/ui/CompositeStatusDisplay.h"

// --- State Tracking ---
uint32_t lastRead = 0;

// --- Hardware Abstraction Layer ---
SensorManager sensorManager;
BatteryMonitor batteryMonitor;
CalibrationManager calibrationManager;
PowerManager powerManager;

// --- Matter Multi-Sensor Cluster ---
MatterMultiSensor matterMultiSensor;

 // -- Matter custome cluster ---
// CustomSoilSensorCluster soilCluster; // Not used

// --- Display Implementation ---
StatusDisplay* statusDisplay;

// --- Setup ---
void setup() {
  Serial.begin(kSerialBaudRate);
  delay(kInitDelay);

  // Initialize display system using factory pattern
  StatusDisplay* primaryDisplay = DisplayFactory::createPrimaryDisplay();
  StatusDisplay* secondaryDisplay = DisplayFactory::createSecondaryDisplay();
  
  if (secondaryDisplay) {
    // Create composite display for dual output (e.g., OLED + Serial when on USB)
    statusDisplay = new CompositeStatusDisplay(primaryDisplay, secondaryDisplay);
  } else {
    // Single display mode
    statusDisplay = primaryDisplay;
  }

  statusDisplay->begin();
  statusDisplay->handleEvent(StatusEvent::Booting);
  statusDisplay->showMessage("Initializing sensor node");

  // Initialize hardware abstraction layer
  calibrationManager.begin();
  powerManager.begin();
  sensorManager.begin();
  batteryMonitor.begin();
  batteryMonitor.setCalibrationManager(&calibrationManager);

  // Start Matter cluster with full integration
  matterMultiSensor.begin();
  matterMultiSensor.setPowerManager(&powerManager);
  matterMultiSensor.setCalibrationManager(&calibrationManager);

}

// --- Loop ---
void loop() {
  uint32_t now = millis();
  
  // Update power state based on current conditions
  float voltage = batteryMonitor.readVoltage();
  bool usbConnected = DisplayFactory::isUsbConnected();
  powerManager.updatePowerState(voltage, usbConnected);
  
  // Check if it's time for a sensor reading
  uint32_t currentSleepInterval = powerManager.getCurrentSleepInterval();
  if (now - lastRead < currentSleepInterval) {
    statusDisplay->update();
    
    // Enter sleep if power management is enabled
    if (powerManager.shouldEnterSleep()) {
      powerManager.enterSleepMode();
      // Device would sleep here and wake up later
    }
    return;
  }
  lastRead = now;

  // Battery monitoring
  BatteryStatus batteryStatus = batteryMonitor.getStatus();
  
  if (batteryStatus != BatteryStatus::Normal) {
    statusDisplay->handleEvent(StatusEvent::BatteryLow);
    char msg[32];
    sprintf(msg, "Battery: %.2fV", voltage);
    statusDisplay->showMessage(msg);
  }
  
  // Show battery status on displays that support it
  statusDisplay->showBattery(voltage, batteryStatus != BatteryStatus::Normal);

  // Soil moisture read
  float moisture = sensorManager.readMoisture();
  statusDisplay->showMoisture(moisture);
  statusDisplay->handleEvent(StatusEvent::MoisturePublished);

  // Matter publishing - update all sensor values
  matterMultiSensor.setSoilMoisture(moisture);
  matterMultiSensor.setBatteryVoltage(voltage);
  matterMultiSensor.setUsbConnected(usbConnected);
  
  // Update calibration values periodically (every 10th reading)
  static uint8_t calibUpdateCounter = 0;
  if (++calibUpdateCounter >= 10) {
    calibUpdateCounter = 0;
    matterMultiSensor.updateCalibrationValues();
  }
  
  if (matterMultiSensor.isOnline()) {
    statusDisplay->handleEvent(StatusEvent::MatterOnline);
  } else {
    statusDisplay->handleEvent(StatusEvent::MatterOffline);
  }

  statusDisplay->update();
}
