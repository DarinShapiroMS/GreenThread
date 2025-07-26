#pragma once
#include <Arduino.h>
#include <stdint.h>

// Forward declarations
class SensorManager;
class BatteryMonitor;
class CalibrationManager;
class PowerManager;

/**
 * Green Thread Soil Sensor Custom Matter Cluster
 * 
 * Generated from Matter SDK ZAP code generation
 * Cluster ID: 0xFFF1FC30 (Vendor: 0xFFF1, Cluster: 0xFC30)
 * 
 * This implements the custom cluster for soil moisture monitoring
 * with advanced calibration and power management features.
 */
class GreenThreadSoilSensorCluster {
public:
    // Cluster and Vendor IDs from generated code
    static const uint32_t CLUSTER_ID = 0xFC30;
    static const uint16_t VENDOR_ID = 0xFFF1;
    static const uint32_t FULL_CLUSTER_ID = 0xFFF1FC30;  // Combined for easy reference
    
    // Attribute IDs (from generated MTRClusterConstants.h)
    enum AttributeId : uint16_t {
        // Primary sensor readings
        ATTR_SOIL_MOISTURE_PERCENT = 0x0000,
        ATTR_SOIL_MOISTURE_RAW = 0x0001,
        ATTR_SOIL_TEMPERATURE_CELSIUS = 0x0002,
        ATTR_AIR_TEMPERATURE_CELSIUS = 0x0003,
        ATTR_HUMIDITY_PERCENT = 0x0004,
        
        // Calibration attributes
        ATTR_CALIBRATION_STATUS = 0x0010,
        ATTR_CALIBRATION_DRY_VALUE = 0x0011,
        ATTR_CALIBRATION_WET_VALUE = 0x0012,
        ATTR_MOISTURE_THRESHOLD_LOW = 0x0013,
        ATTR_MOISTURE_THRESHOLD_HIGH = 0x0014,
        ATTR_CALIBRATION_POINTS_COUNT = 0x0015,
        
        // Power management
        ATTR_BATTERY_VOLTAGE_MV = 0x0020,
        ATTR_BATTERY_LEVEL_PERCENT = 0x0021,
        ATTR_POWER_STATE = 0x0022,
        ATTR_SLEEP_INTERVAL_SECONDS = 0x0023,
        ATTR_MEASUREMENT_INTERVAL_SECONDS = 0x0024,
        
        // System status
        ATTR_SENSOR_STATUS = 0x0030,
        ATTR_LAST_MEASUREMENT_TIME = 0x0031,
        ATTR_MEASUREMENT_COUNT = 0x0032,
        ATTR_ERROR_CODE = 0x0033,
        ATTR_FIRMWARE_VERSION = 0x0034
    };
    
    // Command IDs (from generated code)
    enum CommandId : uint8_t {
        CMD_START_DRY_CALIBRATION = 0x10,
        CMD_START_WET_CALIBRATION = 0x11,
        CMD_RESET_CALIBRATION = 0x12,
        CMD_FORCE_MEASUREMENT = 0x13,
        CMD_SET_THRESHOLDS = 0x14,
        CMD_SET_MEASUREMENT_INTERVAL = 0x15,
        CMD_GET_STATUS = 0x16,
        CMD_ENTER_SLEEP_MODE = 0x17
    };
    
    // Event IDs (from generated code)
    enum EventId : uint8_t {
        EVENT_MOISTURE_THRESHOLD_CROSSED = 0x00,
        EVENT_BATTERY_LEVEL_CHANGED = 0x01,
        EVENT_POWER_STATE_CHANGED = 0x02,
        EVENT_CALIBRATION_COMPLETED = 0x03,
        EVENT_SYSTEM_ERROR = 0x04
    };
    
    // Enums for status values
    enum CalibrationStatus : uint8_t {
        CALIBRATION_NOT_CALIBRATED = 0,
        CALIBRATION_DRY_ONLY = 1,
        CALIBRATION_WET_ONLY = 2,
        CALIBRATION_FULLY_CALIBRATED = 3,
        CALIBRATION_IN_PROGRESS = 4,
        CALIBRATION_ERROR = 5
    };
    
    enum CustomPowerState : uint8_t {
        POWER_ACTIVE = 0,
        POWER_SLEEP = 1,
        POWER_DEEP_SLEEP = 2,
        POWER_CRITICAL_BATTERY = 3
    };
    
    enum SensorStatus : uint8_t {
        SENSOR_OK = 0,
        SENSOR_ERROR = 1,
        SENSOR_DISCONNECTED = 2,
        SENSOR_CALIBRATING = 3,
        SENSOR_WARMING_UP = 4
    };

private:
    // Hardware abstraction references
    SensorManager* sensorManager;
    BatteryMonitor* batteryMonitor;
    CalibrationManager* calibrationManager;
    PowerManager* powerManager;
    
    // Current attribute values
    struct AttributeValues {
        // Sensor readings
        uint8_t soilMoisturePercent = 0;
        uint16_t soilMoistureRaw = 0;
        int16_t soilTemperatureCelsius = 0;
        int16_t airTemperatureCelsius = 0;
        uint8_t humidityPercent = 0;
        
        // Calibration
        uint8_t calibrationStatus = CALIBRATION_NOT_CALIBRATED;
        uint16_t calibrationDryValue = 1023;
        uint16_t calibrationWetValue = 0;
        uint8_t moistureThresholdLow = 20;
        uint8_t moistureThresholdHigh = 80;
        uint8_t calibrationPointsCount = 0;
        
        // Power
        uint16_t batteryVoltageMv = 3300;
        uint8_t batteryLevelPercent = 100;
        uint8_t powerState = POWER_ACTIVE;
        uint16_t sleepIntervalSeconds = 300;  // 5 minutes default
        uint16_t measurementIntervalSeconds = 60;  // 1 minute default
        
        // System
        uint8_t sensorStatus = SENSOR_OK;
        uint32_t lastMeasurementTime = 0;
        uint32_t measurementCount = 0;
        uint8_t errorCode = 0;
        uint32_t firmwareVersion = 0x010000;  // 1.0.0
    } attributes;
    
    // Internal state
    bool clusterInitialized = false;
    uint32_t lastAttributeUpdate = 0;
    uint32_t lastEventSent = 0;
    
    // Event tracking for threshold crossing
    uint8_t lastMoistureLevel = 0;
    bool thresholdCrossedHigh = false;
    bool thresholdCrossedLow = false;

public:
    /**
     * Constructor - takes references to hardware abstraction layers
     */
    GreenThreadSoilSensorCluster(SensorManager* sm, BatteryMonitor* bm, 
                                CalibrationManager* cm, PowerManager* pm);
    
    /**
     * Initialize the cluster - call once in setup()
     */
    bool begin();
    
    /**
     * Update cluster attributes - call regularly in loop()
     * @param forceUpdate - force update even if values haven't changed
     */
    void update(bool forceUpdate = false);
    
    /**
     * Check if Matter/Thread connection is online
     * @return true if device is commissioned and Thread connected
     */
    bool isOnline() const;
    
    // === Attribute Getters ===
    uint8_t getSoilMoisturePercent() const { return attributes.soilMoisturePercent; }
    uint16_t getSoilMoistureRaw() const { return attributes.soilMoistureRaw; }
    int16_t getSoilTemperatureCelsius() const { return attributes.soilTemperatureCelsius; }
    int16_t getAirTemperatureCelsius() const { return attributes.airTemperatureCelsius; }
    uint8_t getHumidityPercent() const { return attributes.humidityPercent; }
    uint8_t getCalibrationStatus() const { return attributes.calibrationStatus; }
    uint16_t getBatteryVoltageMv() const { return attributes.batteryVoltageMv; }
    uint8_t getBatteryLevelPercent() const { return attributes.batteryLevelPercent; }
    uint8_t getPowerState() const { return attributes.powerState; }
    uint8_t getSensorStatus() const { return attributes.sensorStatus; }
    uint32_t getMeasurementCount() const { return attributes.measurementCount; }
    
    // === Command Handlers ===
    bool handleStartDryCalibration();
    bool handleStartWetCalibration();
    bool handleResetCalibration();
    bool handleForceMeasurement();
    bool handleSetThresholds(uint8_t lowThreshold, uint8_t highThreshold);
    bool handleSetMeasurementInterval(uint16_t intervalSeconds);
    bool handleGetStatus();
    bool handleEnterSleepMode();
    
    // === Event Generation ===
    void sendMoistureThresholdCrossedEvent(uint8_t newLevel, uint8_t thresholdType);
    void sendBatteryLevelChangedEvent(uint8_t newLevel);
    void sendPowerStateChangedEvent(uint8_t newState);
    void sendCalibrationCompletedEvent(uint8_t status);
    void sendSystemErrorEvent(uint8_t errorCode);
    
    // === Utility Methods ===
    bool isCalibrated() const { 
        return attributes.calibrationStatus == CALIBRATION_FULLY_CALIBRATED; 
    }
    
    bool needsCalibration() const {
        return attributes.calibrationStatus == CALIBRATION_NOT_CALIBRATED;
    }
    
    bool isBatteryLow() const {
        return attributes.batteryLevelPercent < 20;
    }
    
    bool isSensorHealthy() const {
        return attributes.sensorStatus == SENSOR_OK;
    }
    
    // === Debug and Diagnostics ===
    void printClusterInfo() const;
    void printAttributeValues() const;
    
private:
    // Internal update methods
    void updateSensorReadings();
    void updateBatteryStatus();
    void updateCalibrationStatus();
    void updatePowerStatus();
    void updateSystemStatus();
    
    // Event helpers
    void checkThresholdCrossings();
    void sendEvent(uint8_t eventId, const uint8_t* eventData, size_t dataLength);
    
    // Validation helpers
    bool validateThresholds(uint8_t low, uint8_t high) const;
    bool validateMeasurementInterval(uint16_t interval) const;
};
