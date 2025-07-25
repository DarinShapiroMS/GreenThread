#pragma once
#include <Arduino.h>

// --- Hardware Pin Configuration ---
constexpr uint8_t kMoisturePin       = A0;
constexpr uint8_t kBatteryPin        = A1;

// --- Sensor Configuration ---
constexpr uint32_t kReadIntervalMs   = 30000;  // 30 seconds between readings
constexpr float    kBatteryLowThresh = 3.30;   // Battery low threshold in volts

// --- Moisture Sensor Calibration ---
constexpr int kMoistureSensorDry     = 1023;   // ADC value for dry soil
constexpr int kMoistureSensorWet     = 300;    // ADC value for wet soil

// --- Battery Monitoring ---
constexpr float kBatteryVoltageDivider = 5.0;  // Voltage divider ratio
constexpr float kAdcReference         = 1023.0; // ADC reference value

// --- Display Configuration ---
constexpr uint8_t kOledI2cAddress    = 0x3C;   // OLED display I2C address
constexpr uint32_t kDisplayDetectionTimeout = 500; // ms to wait for I2C detection
constexpr bool kEnableSerialWhenUsbConnected = true; // Enable serial display when USB detected

// --- Timing Configuration ---
constexpr uint32_t kSerialBaudRate   = 115200;
constexpr uint32_t kInitDelay        = 50;     // Initial delay in milliseconds

// --- EEPROM Configuration ---
constexpr uint16_t kEepromCalibrationAddress = 0;    // Start address for calibration data
constexpr uint16_t kEepromMagicNumber        = 0xCAFE; // Magic number to validate EEPROM data
constexpr uint8_t  kEepromVersion           = 1;      // Version for future compatibility

// --- Calibration Defaults ---
constexpr int kDefaultMoistureDry     = 1023;   // Default ADC value for dry soil
constexpr int kDefaultMoistureWet     = 300;    // Default ADC value for wet soil
constexpr float kDefaultBatteryDivider = 5.0;   // Default voltage divider ratio

// --- Power Management Configuration ---
constexpr bool kEnablePowerManagement = true;   // Enable state machine power management

// Sleep Intervals (configurable via Matter attributes)
constexpr uint32_t kNormalSleepInterval   = 25000;  // 25s - Normal battery operation  
constexpr uint32_t kExtendedSleepInterval = 45000;  // 45s - Low battery conservation
constexpr uint32_t kLowPowerSleepInterval = 300000; // 5min - Critical battery emergency
constexpr uint32_t kUsbSleepInterval      = 15000;  // 15s - When USB connected (more responsive)

// Battery Thresholds (configurable via Matter attributes)
constexpr float kBatteryNormalThresh    = 3.30;  // Above: normal operation
constexpr float kBatteryExtendedThresh  = 3.10;  // Below: extended sleep intervals  
constexpr float kBatteryCriticalThresh  = 3.00;  // Below: low power mode
constexpr float kBatteryShutdownThresh  = 2.80;  // Below: protective shutdown

// Power Management Behavior
constexpr uint32_t kMaxSleepInterval    = 900000; // 15min - Maximum sleep time limit
constexpr uint32_t kMinSleepInterval    = 5000;   // 5s - Minimum sleep time limit
constexpr bool kAllowRemoteWakeup       = true;   // Allow Matter commands to wake device
constexpr bool kUsbOverridePowerManagement = true; // Disable deep sleep when USB connected
