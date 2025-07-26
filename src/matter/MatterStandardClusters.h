#pragma once
#include <Arduino.h>
// #include <MatterHumidity.h>  // Temporarily commented out for compilation test

/**
 * Standard Matter Clusters for Home Assistant Compatibility
 * 
 * This implements standard Matter clusters alongside our custom cluster
 * to ensure Home Assistant recognizes the device properly.
 */

class MatterStandardClusters {
public:
    // Standard Matter cluster IDs that Home Assistant understands
    static const uint16_t RELATIVE_HUMIDITY_CLUSTER = 0x0405;
    static const uint16_t POWER_SOURCE_CLUSTER = 0x002F;
    static const uint16_t BASIC_INFORMATION_CLUSTER = 0x0028;
    static const uint16_t DESCRIPTOR_CLUSTER = 0x001D;
    
    // Device type for humidity sensor (Home Assistant recognizes this)
    static const uint16_t HUMIDITY_SENSOR_DEVICE_TYPE = 0x0307;
    
    struct RelativeHumidityAttributes {
        uint16_t measuredValue = 0;  // 0-10000 (0.00% to 100.00%)
        uint16_t minMeasuredValue = 0;
        uint16_t maxMeasuredValue = 10000;
        uint16_t tolerance = 0;
    };
    
    struct PowerSourceAttributes {
        uint8_t status = 1;  // 1 = Active
        uint8_t order = 1;
        uint8_t description = 0;
        uint32_t batVoltage = 3300;  // mV
        uint8_t batPercentRemaining = 200;  // 0-200 (0-100%)
        uint8_t batChargeLevel = 0;  // 0=OK, 1=Warning, 2=Critical
    };
    
    struct BasicInformationAttributes {
        // Device identification (shown during commissioning)
        char vendorName[32] = "Green Thread";
        uint16_t vendorId = 0xFFF1;
        char productName[32] = "Soil Moisture Sensor";
        uint16_t productId = 0x0001;
        char nodeLabel[32] = "Green Thread Soil Sensor";
        char location[32] = "";
        
        // Hardware info
        uint16_t hardwareVersion = 1;
        char hardwareVersionString[32] = "v1.0";
        
        // Software info
        uint32_t softwareVersion = 0x010000;  // 1.0.0
        char softwareVersionString[32] = "1.0.0";
        
        // Manufacturing info
        char manufacturingDate[16] = "2025";
        char partNumber[32] = "GT-SMS-001";
        char serialNumber[32] = "GT001";  // Should be unique per device
        
        // Device type
        uint16_t deviceTypeId = HUMIDITY_SENSOR_DEVICE_TYPE;
        char deviceTypeRevision = 1;
    };
    
private:
    RelativeHumidityAttributes humidityAttrs;
    PowerSourceAttributes powerAttrs;
    BasicInformationAttributes basicInfoAttrs;
    
    // Matter humidity sensor instance
    // MatterHumidity matterHumidity;  // Temporarily commented out
    
public:
    void begin();
    void updateMoisture(float moisturePercent);
    void updateBattery(float voltage, uint8_t percent);
    void setDeviceInfo(const char* serialNumber = nullptr, const char* location = nullptr);
    
    // Getters for Home Assistant
    uint16_t getHumidityMeasuredValue() const { return humidityAttrs.measuredValue; }
    uint8_t getBatteryPercentRemaining() const { return powerAttrs.batPercentRemaining; }
    uint8_t getBatteryChargeLevel() const { return powerAttrs.batChargeLevel; }
    
    // Getters for Basic Information
    const char* getVendorName() const { return basicInfoAttrs.vendorName; }
    const char* getProductName() const { return basicInfoAttrs.productName; }
    const char* getNodeLabel() const { return basicInfoAttrs.nodeLabel; }
    uint16_t getVendorId() const { return basicInfoAttrs.vendorId; }
    uint16_t getProductId() const { return basicInfoAttrs.productId; }
};
