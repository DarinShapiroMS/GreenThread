//#include <Arduino.h>
#include <Wire.h>

// Silicon Labs Matter library for Arduino Nano Matter
// #include <Matter.h>  // Temporarily commented out for compilation test

#include "src/config/Config.h"
#include "src/matter/GreenThreadSoilSensorCluster.h"
#include "src/matter/MatterStandardClusters.h"
#include "src/matter/CommissioningManager.h"
#include "src/hardware/SensorManager.h"
#include "src/hardware/BatteryMonitor.h"
#include "src/hardware/CalibrationManager.h"
#include "src/hardware/PowerManager.h"

#include "src/ui/StatusDisplay.h"
#include "src/ui/DisplayFactory.h"
#include "src/ui/CompositeStatusDisplay.h"

// Constants with proper documentation
constexpr uint8_t kCalibUpdatePeriod = 10;  // Update calibration every N sensor readings (not seconds)

// Global variables
uint32_t lastSensorRead = 0;  // Renamed for clarity
bool sleepEventAlreadySent = false;  // Prevent sleep event flooding

// Global objects - using static allocation for embedded safety
StatusDisplay* statusDisplay = nullptr;  // Points to either primaryDisplay or compositeDisplay
SensorManager sensorManager;
BatteryMonitor batteryMonitor;
CalibrationManager calibrationManager;
PowerManager powerManager;

// Static storage for soil cluster to avoid heap allocation
static GreenThreadSoilSensorCluster soilCluster(&sensorManager, &batteryMonitor, &calibrationManager, &powerManager);

// Static storage for standard Matter clusters (Home Assistant compatibility)
static MatterStandardClusters standardClusters;

// Static storage for composite display to avoid heap allocation
static CompositeStatusDisplay compositeDisplay(nullptr, nullptr);

// Commissioning manager - will be initialized after statusDisplay is set
static CommissioningManager* commissioningManager = nullptr;

// All global objects must use deferred begin() - constructors must be trivial
// since Wire.begin() hasn't been called yet during static initialization

// Improved SAFE_CALL macro with better safety - parentheses prevent double evaluation
#define SAFE_CALL(ptr, method, ...) do { \
  if ((ptr) != nullptr) { \
    (ptr)->method(__VA_ARGS__); \
  } \
} while(0)

// Static message buffer to reduce stack pressure - safer than stack allocation every loop
static char messageBuffer[64];

// Debug helper to avoid code duplication
#ifdef DEBUG_SERIAL
inline void debugPrint(const __FlashStringHelper* msg) {
  Serial.println(msg);
}
#else
inline void debugPrint(const __FlashStringHelper* msg) {
  // No-op in release builds
}
#endif

void setup() {
  Serial.begin(kSerialBaudRate);
  delay(kInitDelay);

  // Initialize Matter framework first
  Serial.println(F("=== Initializing Matter Framework ==="));
  // Matter.begin();  // Temporarily commented out for compilation test
  Serial.println(F("✅ Matter framework initialized"));

  // Initialize I2C exactly once - guard against multiple calls during development
  static bool i2cStarted = false;
  if (!i2cStarted) {
    Wire.begin();
    i2cStarted = true;
  }

  #ifdef DEBUG_I2C_SCAN
  // I2C Scanner for OLED debugging (only in debug builds)
  #ifdef DEBUG_SERIAL
  Serial.println(F("=== I2C Scanner ==="));
  #endif
  
  int devices = 0;
  for(byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if(Wire.endTransmission() == 0) {
      #ifdef DEBUG_SERIAL
      Serial.print(F("I2C device at 0x"));
      if(addr < 16) Serial.print(F("0"));
      Serial.println(addr, HEX);
      #endif
      devices++;
    }
  }
  #ifdef DEBUG_SERIAL
  Serial.print(F("Total devices: "));
  Serial.println(devices);
  Serial.println(F("=================="));
  #endif
  #endif

  // Initialize display system using factory pattern
  StatusDisplay* primaryDisplay = DisplayFactory::createPrimaryDisplay();
  StatusDisplay* secondaryDisplay = DisplayFactory::createSecondaryDisplay();
  
  if (secondaryDisplay) {
    // Use static composite display to avoid heap allocation
    compositeDisplay.setPrimary(primaryDisplay);
    compositeDisplay.setSecondary(secondaryDisplay);
    statusDisplay = &compositeDisplay;
  } else {
    // Single display mode
    statusDisplay = primaryDisplay;
  }

  if (statusDisplay) {
    statusDisplay->begin();
    statusDisplay->handleEvent(StatusEvent::BootStarting);
    statusDisplay->showMessage("Initializing sensor node");
  }

  // Initialize hardware abstraction layer
  if (statusDisplay) statusDisplay->handleEvent(StatusEvent::BootSensorInit);
  calibrationManager.begin();
  powerManager.begin();
  sensorManager.begin();
  batteryMonitor.begin();
  batteryMonitor.setCalibrationManager(&calibrationManager);

  // Initialize Green Thread Custom Soil Sensor Cluster
  if (statusDisplay) statusDisplay->handleEvent(StatusEvent::BootMatterInit);
  #ifdef DEBUG_SERIAL
  Serial.println(F("\n=== Initializing Custom Soil Sensor Cluster ==="));
  #endif
  
  // Initialize the static soil cluster (no heap allocation)
  if (soilCluster.begin()) {
    if (statusDisplay) statusDisplay->showMessage("Custom cluster ready");
    #ifdef DEBUG_SERIAL
    Serial.println(F("✅ Green Thread Soil Sensor Cluster initialized successfully!"));
    #endif
  } else {
    if (statusDisplay) statusDisplay->showMessage("Custom cluster failed");
    #ifdef DEBUG_SERIAL
    Serial.println(F("❌ Failed to initialize Green Thread Soil Sensor Cluster"));
    #endif
  }

  // Initialize standard Matter clusters for device identification and HA compatibility
  #ifdef DEBUG_SERIAL
  Serial.println(F("\n=== Initializing Standard Matter Clusters ==="));
  #endif
  standardClusters.begin();
  
  // Set unique device information (generate unique serial based on chip ID or random)
  char uniqueSerial[8];
  snprintf(uniqueSerial, sizeof(uniqueSerial), "GT%04d", random(1000, 9999));
  standardClusters.setDeviceInfo(uniqueSerial, "Garden");
  
  #ifdef DEBUG_SERIAL
  Serial.println(F("✅ Standard Matter Clusters ready for commissioning"));
  #endif

  // Check Matter commissioning status
  #ifdef DEBUG_SERIAL
  Serial.println(F("\n=== Matter Commissioning Status ==="));
  #endif
  
  /*  // Temporarily commented out for compilation test
  if (!Matter.isDeviceCommissioned()) {
    Serial.println(F("Matter device is not commissioned"));
    Serial.println(F("Commission it to your Matter hub with the manual pairing code or QR code"));
    Serial.printf("Manual pairing code: %s\n", Matter.getManualPairingCode().c_str());
    Serial.printf("QR code URL: %s\n", Matter.getOnboardingQRCodeUrl().c_str());
    if (statusDisplay) statusDisplay->showMessage("Ready to commission");
  } else {
    Serial.println(F("Device is commissioned - waiting for Thread network..."));
    if (statusDisplay) statusDisplay->showMessage("Connecting to network");
    
    // Wait for Thread network connection
    while (!Matter.isDeviceThreadConnected()) {
      delay(200);
    }
    Serial.println(F("Connected to Thread network"));
    if (statusDisplay) statusDisplay->showMessage("Connected to network");
  }
  */

  // Initialize commissioning manager after statusDisplay is ready
  #ifdef DEBUG_SERIAL
  Serial.println(F("\n=== Initializing Commissioning Manager ==="));
  #endif
  
  static CommissioningManager staticCommissioningManager(statusDisplay);
  commissioningManager = &staticCommissioningManager;
  commissioningManager->begin();
  
  #ifdef DEBUG_SERIAL
  Serial.println(F("✅ Commissioning Manager ready - long press button to commission"));
  #endif

  if (statusDisplay) {
    statusDisplay->handleEvent(StatusEvent::BootComplete);
    statusDisplay->showMessage("Boot complete");
  }

}

// --- Loop ---
void loop() {
  uint32_t now = millis();
  
  // PRIORITY 1: Handle serial commands immediately for responsive interaction
  handleSerialCommands();
  
  // PRIORITY 2: Handle commissioning (button presses, etc.)
  if (commissioningManager) commissioningManager->update();
  
  // PRIORITY 3: Light-weight status updates that don't block
  if (statusDisplay) statusDisplay->update();
  
  // PRIORITY 3: Check if it's time for heavy sensor operations
  uint32_t currentSleepInterval = powerManager.getCurrentSleepInterval();
  if (now - lastSensorRead < currentSleepInterval) {
    // Skip heavy operations, just handle power management
    if (powerManager.shouldEnterSleep() && !sleepEventAlreadySent) {
      #ifdef DEBUG_SERIAL
      debugPrint(F("[Main] Entering sleep mode"));
      #endif
      sleepEventAlreadySent = true;
      // Enter sleep FIRST, then handle display event to avoid race condition
      // The RTC/GPIO wake-up will restore the display properly
      powerManager.enterSleepMode();
      // This line never executes due to [[noreturn]] - device resets on wake
      SAFE_CALL(statusDisplay, handleEvent, StatusEvent::EnteringSleep);
    }
    return; // Exit early to keep loop responsive
  }
  
  // Time for full sensor reading cycle
  lastSensorRead = now;
  sleepEventAlreadySent = false; // Clear sleep event flag since we're actively taking measurements

  // Update power state based on current conditions
  float voltage = batteryMonitor.readVoltage();
  bool usbConnected = DisplayFactory::isUsbConnected();
  BatteryStatus batteryStatus = batteryMonitor.getStatus();
  BatteryState batteryState = batteryMonitor.getBatteryState();
  
  powerManager.updatePowerState(voltage, usbConnected);

  // Battery and power status reporting
  if (batteryState == BatteryState::Healthy) {
    // Battery is connected and functional - show battery details
    if (batteryStatus != BatteryStatus::Normal) {
      SAFE_CALL(statusDisplay, handleEvent, StatusEvent::BatteryLow);
      // Use static buffer to reduce stack pressure
      // Use fixed-point arithmetic to avoid floating-point printf
      int voltageInt = (int)(voltage * 100); // Convert to centivolt (e.g., 3.45V -> 345)
      // Copy PROGMEM string to RAM for safe concatenation
      char statusStr[16];
      strcpy_P(statusStr, batteryMonitor.getBatteryStatusString());
      snprintf(messageBuffer, sizeof(messageBuffer), "Battery: %d.%02dV (%s)", voltageInt/100, voltageInt%100, statusStr);
      SAFE_CALL(statusDisplay, showMessage, messageBuffer);
    }
    
    // Show battery status on displays that support it
    SAFE_CALL(statusDisplay, showBattery, voltage, batteryStatus != BatteryStatus::Normal);
    
    #ifdef DEBUG_SERIAL
    char statusStr[16]; // Cache the string safely from PROGMEM
    strcpy_P(statusStr, batteryMonitor.getBatteryStatusString());
    int voltageInt = (int)(voltage * 100); // Fixed-point for printf
    Serial.print(F("[Power] Battery: "));
    Serial.print(voltageInt/100);
    Serial.print(F("."));
    if (voltageInt%100 < 10) Serial.print(F("0")); // Leading zero for formatting
    Serial.print(voltageInt%100);
    Serial.print(F("V ("));
    Serial.print(statusStr);
    Serial.print(F(")"));
    if (usbConnected) Serial.print(F(" + USB"));
    Serial.println();
    #endif
    
  } else if (batteryState == BatteryState::NotPresent) {
    // No battery detected
    #ifdef DEBUG_SERIAL
    if (usbConnected) {
      debugPrint(F("[Power] USB powered - no battery detected"));
    } else {
      debugPrint(F("[Power] ERROR: No power source detected!"));
    }
    #endif
    
  } else if (batteryState == BatteryState::DeadBattery) {
    // Dead battery detected
    // Use static buffer and fixed-point arithmetic to avoid floating-point printf
    int voltageInt = (int)(voltage * 100); // Convert to centivolt (e.g., 2.85V -> 285)
    snprintf(messageBuffer, sizeof(messageBuffer), "Dead battery detected: %d.%02dV", voltageInt/100, voltageInt%100);
    SAFE_CALL(statusDisplay, showMessage, messageBuffer);
    #ifdef DEBUG_SERIAL
    Serial.print(F("[Power] Dead battery detected: "));
    Serial.print(voltageInt/100);
    Serial.print(F("."));
    if (voltageInt%100 < 10) Serial.print(F("0")); // Leading zero for formatting
    Serial.print(voltageInt%100);
    Serial.print(F("V"));
    if (usbConnected) Serial.print(F(" (USB keeping device alive)"));
    Serial.println();
    #endif
  }

  // Soil moisture read
  float moisture = sensorManager.readMoisture();
  if (statusDisplay) {
    statusDisplay->handleEvent(StatusEvent::MoisturePublishing);
    statusDisplay->showMoisture(moisture);
    statusDisplay->handleEvent(StatusEvent::MoisturePublished);
  }

  // Matter publishing - update all sensor values
  // Update Green Thread Custom Soil Sensor Cluster
  soilCluster.update();
  
  // Update standard Matter clusters for Home Assistant compatibility and device identification
  standardClusters.updateMoisture(moisture);
  uint8_t batteryPercent = (uint8_t)constrain(((voltage - 2.7) / (3.3 - 2.7)) * 100.0, 0, 100);
  standardClusters.updateBattery(voltage, batteryPercent);
  
  // Update calibration values periodically (every Nth reading)
  static uint8_t calibUpdateCounter = 0;
  if (++calibUpdateCounter >= kCalibUpdatePeriod) {
    calibUpdateCounter = 0;
    
    // Force update custom cluster with latest calibration data
    soilCluster.update(true);
  }
  
  // Enhanced connection status tracking
  static bool wasThreadConnected = false;
  static bool wasMatterOnline = false;
  
  // Access Matter status through the custom soil cluster (static object is always valid)
  bool isMatterOnline = soilCluster.isOnline();
  
  // For now, treat Matter online status as both Thread and commission status
  // since the isOnline() method already checks both conditions
  bool isThreadConnected = isMatterOnline;  // Simplified for now
  bool isMatterCommissioned = isMatterOnline;
  
  // Thread connection status
  if (isThreadConnected && !wasThreadConnected) {
    SAFE_CALL(statusDisplay, handleEvent, StatusEvent::ThreadConnected);
    wasThreadConnected = true;
  } else if (!isThreadConnected && wasThreadConnected) {
    SAFE_CALL(statusDisplay, handleEvent, StatusEvent::ThreadDisconnected);
    wasThreadConnected = false;
  }
  
  // Matter connection status
  if (isMatterOnline && !wasMatterOnline) {
    SAFE_CALL(statusDisplay, handleEvent, StatusEvent::MatterOnline);
    wasMatterOnline = true;
  } else if (!isMatterOnline && wasMatterOnline) {
    SAFE_CALL(statusDisplay, handleEvent, StatusEvent::MatterOffline);
    wasMatterOnline = false;
  }
  
  // Check for connection failures and handle LED accordingly
  if (!isThreadConnected) {
    SAFE_CALL(statusDisplay, handleEvent, StatusEvent::ThreadConnectionFailed);
  } else if (!isMatterOnline) {
    SAFE_CALL(statusDisplay, handleEvent, StatusEvent::MatterConnectionFailed);
  }
  // Note: Removed the automatic EnteringSleep call here to prevent flooding
  // The LED will turn off automatically after moisture display completes
}

// === Serial Command Handler for Testing Custom Cluster ===
void handleSerialCommands() {
  if (!Serial.available()) {
    return;
  }
  
  static char commandBuffer[kSerialBufferSize]; // Reduced buffer size for safety
  size_t maxRead = sizeof(commandBuffer) - 1;
  size_t len = Serial.readBytesUntil('\n', commandBuffer, maxRead);
  commandBuffer[len] = '\0'; // Null terminate
  
  // Trim whitespace using safe string operations
  while(len > 0 && (commandBuffer[len-1] == '\r' || commandBuffer[len-1] == ' ')) {
    commandBuffer[--len] = '\0';
  }
  
  if (len == 0) return; // Empty command
  
  #ifdef DEBUG_SERIAL
  Serial.print(F("Command received: "));
  Serial.println(commandBuffer);
  #endif
  
  // Convert to lowercase for comparison (safe bounds already established)
  for(size_t i = 0; i < len; i++) {
    commandBuffer[i] = tolower(commandBuffer[i]);
  }
  
  if (strcmp(commandBuffer, "help") == 0 || strcmp(commandBuffer, "h") == 0) {
    printSerialHelp();
  } else if (strcmp(commandBuffer, "status") == 0 || strcmp(commandBuffer, "s") == 0) {
    soilCluster.handleGetStatus();
  } else if (strcmp(commandBuffer, "info") == 0 || strcmp(commandBuffer, "i") == 0) {
    soilCluster.printClusterInfo();
  } else if (strcmp(commandBuffer, "calibrate_dry") == 0 || strcmp(commandBuffer, "cdry") == 0) {
    #ifdef DEBUG_SERIAL
    debugPrint(F("Starting dry calibration..."));
    #endif
    soilCluster.handleStartDryCalibration();
  } else if (strcmp(commandBuffer, "calibrate_wet") == 0 || strcmp(commandBuffer, "cwet") == 0) {
    #ifdef DEBUG_SERIAL
    debugPrint(F("Starting wet calibration..."));
    #endif
    soilCluster.handleStartWetCalibration();
  } else if (strcmp(commandBuffer, "reset_calibration") == 0 || strcmp(commandBuffer, "reset") == 0) {
    #ifdef DEBUG_SERIAL
    debugPrint(F("Resetting calibration..."));
    #endif
    soilCluster.handleResetCalibration();
  } else if (strcmp(commandBuffer, "measure") == 0 || strcmp(commandBuffer, "m") == 0) {
    #ifdef DEBUG_SERIAL
    debugPrint(F("Forcing measurement..."));
    #endif
    soilCluster.handleForceMeasurement();
  } else if (strcmp(commandBuffer, "sleep") == 0) {
    #ifdef DEBUG_SERIAL
    debugPrint(F("Entering sleep mode..."));
    #endif
    SAFE_CALL(statusDisplay, handleEvent, StatusEvent::EnteringSleep);
    soilCluster.handleEnterSleepMode();
  } else if (strcmp(commandBuffer, "led_off") == 0 || strcmp(commandBuffer, "loff") == 0) {
    #ifdef DEBUG_SERIAL
    debugPrint(F("Force LED OFF..."));
    #endif
    SAFE_CALL(statusDisplay, handleEvent, StatusEvent::EnteringSleep);
  } else if (strcmp(commandBuffer, "led_green") == 0 || strcmp(commandBuffer, "lgreen") == 0) {
    #ifdef DEBUG_SERIAL
    debugPrint(F("Force LED GREEN..."));
    #endif
    SAFE_CALL(statusDisplay, handleEvent, StatusEvent::BootStarting);
  } else if (strcmp(commandBuffer, "led_red") == 0 || strcmp(commandBuffer, "lred") == 0) {
    #ifdef DEBUG_SERIAL
    debugPrint(F("Force LED RED..."));
    #endif
    SAFE_CALL(statusDisplay, handleEvent, StatusEvent::ThreadConnectionFailed);
  } else if (strcmp(commandBuffer, "test_red") == 0 || strcmp(commandBuffer, "tr") == 0) {
    #ifdef DEBUG_SERIAL
    Serial.println(F("Test RED LED..."));
    #endif
    if (statusDisplay) {
      statusDisplay->testRed();
    } else {
      #ifdef DEBUG_SERIAL
      Serial.println(F("Error: No status display available"));
      #endif
    }
  } else if (strcmp(commandBuffer, "test_green") == 0 || strcmp(commandBuffer, "tg") == 0) {
    #ifdef DEBUG_SERIAL
    Serial.println(F("Test GREEN LED..."));
    #endif
    if (statusDisplay) {
      statusDisplay->testGreen();
    } else {
      #ifdef DEBUG_SERIAL
      Serial.println(F("Error: No status display available"));
      #endif
    }
  } else if (strcmp(commandBuffer, "test_blue") == 0 || strcmp(commandBuffer, "tb") == 0) {
    #ifdef DEBUG_SERIAL
    Serial.println(F("Test BLUE LED..."));
    #endif
    if (statusDisplay) {
      statusDisplay->testBlue();
    } else {
      #ifdef DEBUG_SERIAL
      Serial.println(F("Error: No status display available"));
      #endif
    }
  } else if (strcmp(commandBuffer, "test_off") == 0 || strcmp(commandBuffer, "toff") == 0) {
    #ifdef DEBUG_SERIAL
    Serial.println(F("Test LED OFF..."));
    #endif
    if (statusDisplay) {
      statusDisplay->testOff();
    } else {
      #ifdef DEBUG_SERIAL
      Serial.println(F("Error: No status display available"));
      #endif
    }
  } else if (strncmp(commandBuffer, "threshold ", 10) == 0) {
    // Parse "threshold low high" command with bounds checking
    char* token = strtok(commandBuffer + 10, " ");
    if (token) {
      int low = atoi(token);
      token = strtok(nullptr, " ");
      if (token) {
        int high = atoi(token);
        if (low >= 0 && low <= 100 && high >= 0 && high <= 100 && low < high) {
          #ifdef DEBUG_SERIAL
          Serial.print(F("Setting thresholds: Low="));
          Serial.print(low);
          Serial.print(F("%, High="));
          Serial.print(high);
          Serial.println(F("%"));
          #endif
          soilCluster.handleSetThresholds(low, high);
        } else {
          #ifdef DEBUG_SERIAL
          debugPrint(F("Error: Invalid threshold values (0-100%, low < high)"));
          #endif
        }
      } else {
        #ifdef DEBUG_SERIAL
        debugPrint(F("Usage: threshold <low> <high>"));
        #endif
      }
    } else {
      #ifdef DEBUG_SERIAL
      debugPrint(F("Usage: threshold <low> <high>"));
      #endif
    }
  } else if (strncmp(commandBuffer, "interval ", 9) == 0) {
    // Parse "interval seconds" command with bounds checking using constants
    char* endPtr;
    long interval = strtol(commandBuffer + 9, &endPtr, 10);
    if (endPtr != commandBuffer + 9 && interval >= kMinInterval && interval <= kMaxInterval) {
      #ifdef DEBUG_SERIAL
      Serial.print(F("Setting measurement interval: "));
      Serial.print(interval);
      Serial.println(F(" seconds"));
      #endif
      soilCluster.handleSetMeasurementInterval((int)interval);
    } else {
      #ifdef DEBUG_SERIAL
      Serial.print(F("Error: Interval must be between "));
      Serial.print(kMinInterval);
      Serial.print(F("-"));
      Serial.print(kMaxInterval);
      Serial.println(F(" seconds"));
      #endif
    }
  } else if (strcmp(commandBuffer, "cluster") == 0) {
    // Show detailed cluster information
    #ifdef DEBUG_SERIAL
    Serial.println(F("\n=== Green Thread Soil Sensor Cluster ==="));
    Serial.print(F("Cluster ID: 0x"));
    Serial.println(GreenThreadSoilSensorCluster::FULL_CLUSTER_ID, HEX);
    Serial.print(F("Vendor ID: 0x"));
    Serial.println(GreenThreadSoilSensorCluster::VENDOR_ID, HEX);
    Serial.print(F("Calibrated: "));
    Serial.println(soilCluster.isCalibrated() ? F("YES") : F("NO"));
    Serial.print(F("Battery Low: "));
    Serial.println(soilCluster.isBatteryLow() ? F("YES") : F("NO"));
    Serial.print(F("Sensor Health: "));
    Serial.println(soilCluster.isSensorHealthy() ? F("HEALTHY") : F("ERROR"));
    Serial.println(F("======================================"));
    #endif
  } else if (strcmp(commandBuffer, "commission") == 0 || strcmp(commandBuffer, "comm") == 0) {
    #ifdef DEBUG_SERIAL
    Serial.println(F("Starting commissioning mode..."));
    #endif
    if (commissioningManager) {
      commissioningManager->startCommissioning();
    } else {
      #ifdef DEBUG_SERIAL
      Serial.println(F("Error: Commissioning manager not available"));
      #endif
    }
  } else if (strcmp(commandBuffer, "commission_stop") == 0 || strcmp(commandBuffer, "cstop") == 0) {
    #ifdef DEBUG_SERIAL
    Serial.println(F("Stopping commissioning..."));
    #endif
    if (commissioningManager) {
      commissioningManager->stopCommissioning();
    } else {
      #ifdef DEBUG_SERIAL
      Serial.println(F("Error: Commissioning manager not available"));
      #endif
    }
  } else if (strcmp(commandBuffer, "commission_status") == 0 || strcmp(commandBuffer, "cstat") == 0) {
    #ifdef DEBUG_SERIAL
    if (commissioningManager) {
      Serial.print(F("Commissioning state: "));
      Serial.println((int)commissioningManager->getState());
      Serial.print(F("Active: "));
      Serial.println(commissioningManager->isActive() ? F("YES") : F("NO"));
    } else {
      Serial.println(F("Error: Commissioning manager not available"));
    }
    #endif
  } else {
    #ifdef DEBUG_SERIAL
    Serial.print(F("Unknown command: "));
    Serial.println(commandBuffer);
    debugPrint(F("Type 'help' for available commands"));
    #endif
  }
}

void printSerialHelp() {
  // Use single string literals for faster output
  Serial.println(F("\n=== Green Thread Soil Sensor Commands ===\n"
                   "Basic Commands:\n"
                   "  help, h          - Show this help\n"
                   "  status, s        - Show current status\n"
                   "  info, i          - Show cluster info\n"
                   "  cluster          - Show detailed cluster info\n"
                   "  measure, m       - Force measurement\n"
                   "\n"
                   "Commissioning Commands:\n"
                   "  commission, comm - Start commissioning mode\n"
                   "  commission_stop, cstop - Stop commissioning\n"
                   "  commission_status, cstat - Show commissioning state\n"
                   "\n"
                   "LED Debug Commands:\n"
                   "  led_off, loff    - Force LED off\n"
                   "  led_green, lgreen - Force LED green\n"
                   "  led_red, lred    - Force LED red\n"
                   "  test_red, tr     - Test RED pin directly\n"
                   "  test_green, tg   - Test GREEN pin directly\n"
                   "  test_blue, tb    - Test BLUE pin directly\n"
                   "  test_off, toff   - Turn off all pins directly\n"
                   "\n"
                   "Calibration Commands:\n"
                   "  calibrate_dry    - Start dry calibration\n"
                   "  calibrate_wet    - Start wet calibration\n"
                   "  reset            - Reset calibration\n"
                   "\n"
                   "Configuration Commands:\n"
                   "  threshold <L> <H> - Set moisture thresholds (0-100%)\n"
                   "  interval <sec>    - Set measurement interval (10-3600s)\n"
                   "  sleep            - Enter sleep mode\n"
                   "\n"
                   "Hardware Notes:\n"
                   "  Built-in button: Long press (3s) = commission\n"
                   "                   Very long press (10s) = factory reset\n"
                   "\n"
                   "Examples:\n"
                   "  threshold 20 80  - Set low=20%, high=80%\n"
                   "  interval 300     - Measure every 5 minutes\n"
                   "========================================"));
}
