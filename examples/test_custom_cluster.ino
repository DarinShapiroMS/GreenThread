/**
 * Green Thread Soil Sensor Cluster - Test and Demo
 * 
 * This file demonstrates how to use the custom Matter cluster
 * and provides examples of all the functionality.
 */

#include "src/matter/GreenThreadSoilSensorCluster.h"
#include "src/hardware/SensorManager.h"
#include "src/hardware/BatteryMonitor.h"
#include "src/hardware/CalibrationManager.h"
#include "src/hardware/PowerManager.h"

// Hardware components (would be initialized in main sketch)
SensorManager testSensorManager;
BatteryMonitor testBatteryMonitor;
CalibrationManager testCalibrationManager;
PowerManager testPowerManager;

// Custom cluster instance
GreenThreadSoilSensorCluster testCluster(&testSensorManager, &testBatteryMonitor, 
                                         &testCalibrationManager, &testPowerManager);

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== Green Thread Soil Sensor Cluster Test ===");
  
  // Initialize hardware components
  testSensorManager.begin();
  testBatteryMonitor.begin();
  testCalibrationManager.begin();
  testPowerManager.begin();
  
  // Initialize custom cluster
  if (testCluster.begin()) {
    Serial.println("✅ Custom cluster initialized successfully!");
  } else {
    Serial.println("❌ Custom cluster initialization failed!");
    return;
  }
  
  // Show cluster information
  testCluster.printClusterInfo();
  
  // Test all commands
  Serial.println("\n=== Testing Cluster Commands ===");
  
  // Test status command
  Serial.println("\n1. Getting initial status:");
  testCluster.handleGetStatus();
  
  // Test forced measurement
  Serial.println("\n2. Testing forced measurement:");
  testCluster.handleForceMeasurement();
  
  // Test threshold setting
  Serial.println("\n3. Testing threshold configuration:");
  testCluster.handleSetThresholds(25, 75);
  
  // Test measurement interval
  Serial.println("\n4. Testing measurement interval:");
  testCluster.handleSetMeasurementInterval(120);  // 2 minutes
  
  // Test calibration commands
  Serial.println("\n5. Testing calibration commands:");
  Serial.println("Starting dry calibration...");
  testCluster.handleStartDryCalibration();
  delay(2000);  // Simulate calibration time
  
  Serial.println("Starting wet calibration...");
  testCluster.handleStartWetCalibration();
  delay(2000);  // Simulate calibration time
  
  // Show final status
  Serial.println("\n6. Final status after tests:");
  testCluster.handleGetStatus();
  
  Serial.println("\n=== Test completed! ===");
  Serial.println("Available serial commands:");
  Serial.println("  status   - Show current status");
  Serial.println("  measure  - Force measurement");
  Serial.println("  info     - Show cluster info");
  Serial.println("  help     - Show all commands");
}

void loop() {
  // Regular cluster updates
  testCluster.update();
  
  // Handle serial commands (same as in main sketch)
  handleTestCommands();
  
  delay(1000);  // Update every second for testing
}

void handleTestCommands() {
  if (!Serial.available()) {
    return;
  }
  
  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toLowerCase();
  
  Serial.print("\nCommand: ");
  Serial.println(command);
  
  if (command == "status") {
    testCluster.handleGetStatus();
  } else if (command == "measure") {
    testCluster.handleForceMeasurement();
  } else if (command == "info") {
    testCluster.printClusterInfo();
  } else if (command == "calibrate_dry") {
    testCluster.handleStartDryCalibration();
  } else if (command == "calibrate_wet") {
    testCluster.handleStartWetCalibration();
  } else if (command == "reset") {
    testCluster.handleResetCalibration();
  } else if (command == "help") {
    Serial.println("\nAvailable commands:");
    Serial.println("  status - Show current values");
    Serial.println("  measure - Force measurement");
    Serial.println("  info - Show cluster info");
    Serial.println("  calibrate_dry - Start dry calibration");
    Serial.println("  calibrate_wet - Start wet calibration");
    Serial.println("  reset - Reset calibration");
    Serial.println("  help - Show this help");
  } else if (command != "") {
    Serial.print("Unknown command: ");
    Serial.println(command);
    Serial.println("Type 'help' for available commands");
  }
}
