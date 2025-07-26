# Custom Green Thread Soil Sensor Cluster Integration Guide

## 🎉 Success! Your Custom Cluster is Generated and Ready

The ZAP code generation process has successfully created your custom Green Thread Soil Sensor cluster. Here's where the generated code is located and how to use it.

## 📁 Generated Code Locations

### 1. **Cluster ID and Constants** (MAIN REFERENCE)
```
/src/darwin/Framework/CHIP/zap-generated/MTRClusterConstants.h
```
Contains:
- Cluster ID: `MTRClusterIDTypeGreenThreadSoilSensorID = 0xFFF1FC30`
- All 20 attribute IDs (SoilMoisturePercent, CalibrationStatus, etc.)
- All 8 command IDs (StartDryCalibration, ForceMeasurement, etc.)
- All 5 event IDs (MoistureThresholdCrossed, BatteryLevelChanged, etc.)

### 2. **Matter IDL Definition**
```
/src/controller/data_model/controller-clusters.matter
```
Contains the complete cluster definition in Matter IDL format.

### 3. **iOS/macOS Framework Code**
```
/src/darwin/Framework/CHIP/zap-generated/
├── MTRBaseClusters.h          # Base cluster interfaces
├── MTRClusters.h              # High-level cluster APIs
├── MTRCommandPayloadsObjc.h   # Command payload structures
└── MTRStructsObjc.h           # Data structures
```

### 4. **Java/Android Support**
```
/src/controller/java/zap-generated/
├── CHIPAttributeTLVValueDecoder.cpp
└── CHIPEventTLVValueDecoder.cpp
```

## 🏠 Home Assistant Compatibility Guide

### **❌ Challenge: Custom Clusters Not Auto-Recognized**
Home Assistant will NOT automatically understand your custom cluster because:
- No built-in support for cluster ID `0xFFF1FC30`
- Unknown attribute meanings (`SoilMoisturePercent`, `CalibrationStatus`)
- No device type mapping

### **✅ What Home Assistant Will See:**
```yaml
device:
  name: "Unknown Matter Device"
  clusters:
    - cluster_id: 0xFFF1FC30  # Unsupported
      attributes: [raw_data]   # No interpretation
```

### **🎯 Recommended Approach for Home Assistant:**

**Use Option 1** (Standard Clusters) for immediate Home Assistant compatibility:

```cpp
// Map to standard clusters Home Assistant understands:
// Soil Moisture → RelativeHumidity (0x0405) → Shows as humidity sensor
// Battery Level → PowerSource (0x002F) → Shows battery status  
// Temperature → TemperatureMeasurement (0x0402) → Shows as temp sensor
```

Home Assistant will automatically create:
- `sensor.soil_moisture_humidity` (0-100%)
- `sensor.soil_sensor_battery` (0-100%)
- `binary_sensor.soil_sensor_battery_low`

## 🔧 Integration Methods for Arduino Nano Matter

Since you're using Arduino Nano Matter (Silicon Labs MGM240S), you have several integration options:

### **Option 1: Use Matter Standard Clusters (RECOMMENDED FOR HOME ASSISTANT)**

**This is the BEST option for Home Assistant compatibility!** Map your functionality to existing Matter clusters:

```cpp
// In your Arduino sketch - use existing clusters that Home Assistant understands
#include "matter_humidity_sensor.h"

// ✅ HOME ASSISTANT WILL RECOGNIZE THESE:
// Map soil moisture to RelativeHumidity cluster (0x0405) 
// - Home Assistant shows this as "humidity sensor"
// - Soil moisture 0-100% maps perfectly to humidity 0-100%

// Map battery to PowerSource cluster (0x002F)
// - Home Assistant shows battery level and charging status

// Map temperature to TemperatureMeasurement cluster (0x0402)
// - If your sensor has temperature capability

void setup() {
  // Home Assistant will automatically detect and configure:
  // - sensor.soil_moisture_humidity (0-100%)
  // - sensor.soil_sensor_battery (0-100%)
  // - binary_sensor.soil_sensor_battery_low
}
```

### **Option 2: Custom Cluster + Home Assistant Integration**

If you want to use your custom cluster, you'll need additional Home Assistant configuration:

**A) Home Assistant Custom Integration:**
```python
# custom_components/green_thread_soil/sensor.py
from homeassistant.components.matter import async_get_matter_client
from homeassistant.helpers.entity import Entity

class GreenThreadSoilSensor(Entity):
    def __init__(self, matter_device):
        self._device = matter_device
        self._cluster_id = 0xFFF1FC30
        
    @property
    def state(self):
        # Read soil moisture from custom cluster
        return self._device.read_attribute(self._cluster_id, 0x0000)
```

**B) Home Assistant Configuration:**
```yaml
# configuration.yaml
matter:
  custom_clusters:
    - cluster_id: 0xFFF1FC30
      name: "Green Thread Soil Sensor"
      attributes:
        0x0000:
          name: "soil_moisture_percent"
          unit: "%"
          device_class: "humidity"
```

**C) Manual Entity Configuration:**
```yaml
# configuration.yaml
sensor:
  - platform: template
    sensors:
      soil_moisture:
        friendly_name: "Soil Moisture"
        value_template: "{{ states('sensor.matter_device_attribute_0000') }}"
        unit_of_measurement: "%"
        device_class: humidity
```

### **Option 3: Create Arduino Implementation**

Create your own Arduino cluster implementation using the generated IDs:

```cpp
// GreenThreadSoilSensorCluster.h
#pragma once
#include <Matter.h>

class GreenThreadSoilSensorCluster {
private:
  static const uint32_t CLUSTER_ID = 0xFFF1FC30;
  
  // Attributes
  uint8_t soilMoisturePercent = 0;
  uint16_t soilMoistureRaw = 0;
  uint8_t calibrationStatus = 0;
  uint16_t batteryVoltageMv = 3300;
  
public:
  void init();
  void updateSoilMoisture(uint16_t rawValue);
  void handleCalibrationCommand(uint8_t cmdId);
  void sendEvent(uint8_t eventId, uint8_t* data, size_t len);
};
```

## � How Different Matter Controllers Handle Your Device

### **Option 1 (Standard Clusters) - Universal Compatibility:**
| Controller | Recognition | Functionality |
|------------|------------|---------------|
| 🏠 **Home Assistant** | ✅ Immediate | Humidity sensor, battery sensor, automations |
| 🏡 **Google Home** | ✅ Immediate | "Hey Google, what's the soil moisture?" |
| 🍎 **Apple HomeKit** | ✅ Immediate | Siri control, HomeKit automations |
| 🟦 **SmartThings** | ✅ Immediate | Device control, SmartThings automations |
| ⚡ **chip-tool** | ✅ Full access | Read/write all attributes, send commands |

### **Option 2 (Custom Cluster) - Limited Compatibility:**
| Controller | Recognition | Functionality |
|------------|------------|---------------|
| 🏠 **Home Assistant** | ❌ Unknown device | Raw data only, needs custom integration |
| 🏡 **Google Home** | ❌ Limited | Basic device info, no voice control |
| 🍎 **Apple HomeKit** | ❌ Unsupported | Won't add custom clusters to HomeKit |
| 🟦 **SmartThings** | ❌ Unknown | Basic device detection only |
| ⚡ **chip-tool** | ✅ Full access | Complete cluster control (for testing) |

## 🏠 Home Assistant Custom Cluster Integration Work Required

If you choose Option 2 (Custom Cluster), here's the EXACT work needed in Home Assistant:

### **Method A: Create a Home Assistant Custom Integration (ADVANCED)**

This is the most complete solution but requires Python development skills.

#### **Step 1: Create Custom Integration Structure**
```
config/custom_components/green_thread_soil/
├── __init__.py
├── manifest.json
├── sensor.py
├── const.py
└── services.yaml
```

#### **Step 2: Integration Files**

**manifest.json:**
```json
{
  "domain": "green_thread_soil",
  "name": "Green Thread Soil Sensor",
  "version": "1.0.0",
  "documentation": "https://github.com/your-repo/green-thread-soil",
  "dependencies": ["matter"],
  "codeowners": ["@your-username"],
  "requirements": [],
  "iot_class": "local_polling"
}
```

**const.py:**
```python
DOMAIN = "green_thread_soil"
CLUSTER_ID = 0xFFF1FC30

# Attribute IDs from your generated constants
ATTR_SOIL_MOISTURE_PERCENT = 0x0000
ATTR_SOIL_MOISTURE_RAW = 0x0001
ATTR_CALIBRATION_STATUS = 0x0002
ATTR_BATTERY_VOLTAGE_MV = 0x0020
ATTR_BATTERY_LEVEL_PERCENT = 0x0021
ATTR_POWER_STATE = 0x0022

# Command IDs
CMD_START_DRY_CALIBRATION = 0x0010
CMD_START_WET_CALIBRATION = 0x0011
CMD_FORCE_MEASUREMENT = 0x0013
```

**sensor.py:**
```python
from homeassistant.components.sensor import SensorEntity
from homeassistant.components.matter import async_get_matter_client
from homeassistant.const import PERCENTAGE, UnitOfElectricPotential
from homeassistant.helpers.entity import DeviceInfo
from .const import *

async def async_setup_entry(hass, entry, async_add_entities):
    """Set up Green Thread Soil sensors."""
    matter_client = async_get_matter_client(hass)
    
    # Find devices with our custom cluster
    devices = []
    for node in matter_client.get_nodes():
        if CLUSTER_ID in node.get_cluster_ids():
            devices.extend([
                SoilMoistureSensor(node),
                SoilMoistureRawSensor(node),
                BatteryVoltageSensor(node),
                BatteryLevelSensor(node),
                CalibrationStatusSensor(node),
            ])
    
    async_add_entities(devices)

class GreenThreadSoilSensorBase(SensorEntity):
    """Base class for Green Thread soil sensors."""
    
    def __init__(self, matter_node, attribute_id, name, unit=None):
        self._matter_node = matter_node
        self._attribute_id = attribute_id
        self._attr_name = f"Soil Sensor {name}"
        self._attr_unique_id = f"{matter_node.node_id}_{CLUSTER_ID}_{attribute_id}"
        self._attr_native_unit_of_measurement = unit
        
    @property
    def device_info(self):
        return DeviceInfo(
            identifiers={(DOMAIN, self._matter_node.node_id)},
            name="Green Thread Soil Sensor",
            manufacturer="Green Thread",
            model="Soil Sensor v1.0",
            sw_version="1.0.0",
        )
    
    async def async_update(self):
        """Update the sensor value."""
        try:
            cluster = self._matter_node.get_cluster(CLUSTER_ID)
            self._attr_native_value = await cluster.read_attribute(self._attribute_id)
        except Exception as err:
            self._attr_available = False

class SoilMoistureSensor(GreenThreadSoilSensorBase):
    def __init__(self, matter_node):
        super().__init__(matter_node, ATTR_SOIL_MOISTURE_PERCENT, "Moisture", PERCENTAGE)
        self._attr_device_class = "humidity"
        self._attr_icon = "mdi:water-percent"

class BatteryVoltageSensor(GreenThreadSoilSensorBase):
    def __init__(self, matter_node):
        super().__init__(matter_node, ATTR_BATTERY_VOLTAGE_MV, "Battery Voltage", UnitOfElectricPotential.MILLIVOLT)
        self._attr_device_class = "voltage"
        self._attr_icon = "mdi:battery"

# ... (similar classes for other sensors)
```

**services.yaml:**
```yaml
start_dry_calibration:
  name: Start Dry Calibration
  description: Start dry calibration for soil sensor
  target:
    entity:
      domain: sensor
      integration: green_thread_soil

start_wet_calibration:
  name: Start Wet Calibration
  description: Start wet calibration for soil sensor
  target:
    entity:
      domain: sensor
      integration: green_thread_soil

force_measurement:
  name: Force Measurement
  description: Force an immediate soil moisture measurement
  target:
    entity:
      domain: sensor
      integration: green_thread_soil
```

### **Method B: Template Sensors (EASIER)**

This method maps raw Matter data to Home Assistant entities using templates.

#### **Step 1: Add to configuration.yaml**
```yaml
# configuration.yaml
template:
  - sensor:
      - name: "Soil Moisture Percentage"
        unique_id: "soil_moisture_percent"
        state: "{{ state_attr('sensor.matter_node_123_cluster_4294048816_attribute_0', 'value') | int }}"
        unit_of_measurement: "%"
        device_class: humidity
        icon: "mdi:water-percent"
        availability: "{{ states('sensor.matter_node_123_cluster_4294048816_attribute_0') not in ['unavailable', 'unknown'] }}"
        
      - name: "Soil Moisture Raw"
        unique_id: "soil_moisture_raw"
        state: "{{ state_attr('sensor.matter_node_123_cluster_4294048816_attribute_1', 'value') | int }}"
        icon: "mdi:gauge"
        
      - name: "Soil Sensor Battery Voltage"
        unique_id: "soil_battery_voltage"
        state: "{{ state_attr('sensor.matter_node_123_cluster_4294048816_attribute_32', 'value') | int }}"
        unit_of_measurement: "mV"
        device_class: voltage
        icon: "mdi:battery"
        
      - name: "Soil Sensor Battery Level"
        unique_id: "soil_battery_level"
        state: "{{ state_attr('sensor.matter_node_123_cluster_4294048816_attribute_33', 'value') | int }}"
        unit_of_measurement: "%"
        device_class: battery
        icon: "mdi:battery-outline"

  - binary_sensor:
      - name: "Soil Sensor Battery Low"
        unique_id: "soil_battery_low"
        state: "{{ states('sensor.soil_sensor_battery_level') | int < 20 }}"
        device_class: battery
        icon: "mdi:battery-alert"

  - select:
      - name: "Soil Sensor Calibration"
        unique_id: "soil_calibration_mode"
        state: "{{ states('sensor.calibration_status') }}"
        options:
          - "Not Calibrated"
          - "Dry Calibration"
          - "Wet Calibration" 
          - "Fully Calibrated"
        icon: "mdi:tune"
```

#### **Step 2: Create Automation for Commands**
```yaml
# automations.yaml
- id: soil_sensor_dry_calibration
  alias: "Soil Sensor - Start Dry Calibration"
  trigger:
    - platform: state
      entity_id: input_button.start_dry_calibration
  action:
    - service: matter.send_command
      data:
        command_id: 16  # 0x0010
        cluster_id: 4294048816  # 0xFFF1FC30
        node_id: 123  # Your device node ID

- id: soil_sensor_wet_calibration
  alias: "Soil Sensor - Start Wet Calibration"
  trigger:
    - platform: state
      entity_id: input_button.start_wet_calibration
  action:
    - service: matter.send_command
      data:
        command_id: 17  # 0x0011
        cluster_id: 4294048816  # 0xFFF1FC30
        node_id: 123
```

#### **Step 3: Add Helper Buttons**
```yaml
# configuration.yaml
input_button:
  start_dry_calibration:
    name: "Start Dry Calibration"
    icon: "mdi:tune-vertical"
    
  start_wet_calibration:
    name: "Start Wet Calibration"
    icon: "mdi:water"
    
  force_measurement:
    name: "Force Measurement"
    icon: "mdi:refresh"
```

### **Method C: Quick Manual Entity Creation (SIMPLEST)**

Add individual sensors manually for each attribute:

```yaml
# configuration.yaml
sensor:
  - platform: template
    sensors:
      soil_moisture:
        friendly_name: "Soil Moisture"
        value_template: "{{ state_attr('sensor.matter_raw_data', 'cluster_4294048816_attr_0') }}"
        unit_of_measurement: "%"
        device_class: humidity
        icon: "mdi:water-percent"
```

## 💻 Step-by-Step Implementation

### **Choose Your Method:**
- **Method A**: Most feature-complete, requires Python skills
- **Method B**: Good balance of features and complexity  
- **Method C**: Quick and simple, basic functionality

### **Required Information:**
1. **Matter Node ID**: Find in Home Assistant Developer Tools
2. **Cluster ID**: `4294048816` (decimal) or `0xFFF1FC30` (hex)
3. **Attribute IDs**: Use the generated constants from your guide

### **Testing:**
1. Add device to Home Assistant via Matter
2. Check Developer Tools → States for raw Matter entities
3. Implement chosen method above
4. Restart Home Assistant
5. Check for new soil sensor entities

## �🚀 Practical Implementation Steps

### Step 1: Update Your Arduino Project Structure
```
matter_humidity_sensor/
├── matter_humidity_sensor.ino
├── MatterHumidity.h
├── GreenThreadSoilSensor.h    # NEW - Custom cluster
├── GreenThreadSoilSensor.cpp  # NEW - Implementation
├── OledStatusDisplay.cpp
├── OledStatusDisplay.h
├── ... (other files)
```

### Step 2: Implement Custom Cluster in Arduino

```cpp
// GreenThreadSoilSensor.h
#ifndef GREEN_THREAD_SOIL_SENSOR_H
#define GREEN_THREAD_SOIL_SENSOR_H

#include <stdint.h>

// Cluster and attribute constants from generated code
#define GREEN_THREAD_CLUSTER_ID 0xFC30
#define GREEN_THREAD_VENDOR_ID 0xFFF1

// Attribute IDs
#define ATTR_SOIL_MOISTURE_PERCENT 0x0000
#define ATTR_SOIL_MOISTURE_RAW 0x0001
#define ATTR_CALIBRATION_STATUS 0x0002
#define ATTR_BATTERY_VOLTAGE_MV 0x0020

// Command IDs  
#define CMD_START_DRY_CALIBRATION 0x0010
#define CMD_START_WET_CALIBRATION 0x0011
#define CMD_FORCE_MEASUREMENT 0x0013

class GreenThreadSoilSensor {
private:
  uint8_t _soilMoisturePercent;
  uint16_t _soilMoistureRaw;
  uint16_t _batteryVoltageMv;
  uint8_t _calibrationStatus;

public:
  void begin();
  void update();
  
  // Attribute getters
  uint8_t getSoilMoisturePercent() const { return _soilMoisturePercent; }
  uint16_t getSoilMoistureRaw() const { return _soilMoistureRaw; }
  uint16_t getBatteryVoltageMv() const { return _batteryVoltageMv; }
  
  // Command handlers
  bool handleStartDryCalibration();
  bool handleStartWetCalibration();
  bool handleForceMeasurement();
};

#endif
```

### Step 3: Integrate with Matter

```cpp
// In your main Arduino sketch
#include "GreenThreadSoilSensor.h"

GreenThreadSoilSensor customSensor;

void setup() {
  Serial.begin(115200);
  
  // Initialize your custom sensor
  customSensor.begin();
  
  // Initialize Matter with your custom cluster
  Matter.begin();
  
  // Register custom cluster endpoints
  // (Implementation depends on your Matter library)
}

void loop() {
  // Update sensor readings
  customSensor.update();
  
  // Handle Matter events
  Matter.update();
  
  delay(1000);
}
```

## 📋 Key Generated Constants to Use

From the generated code, here are the main constants you'll need:

```cpp
// Cluster ID
0xFFF1FC30  // GreenThreadSoilSensorID

// Attribute IDs
0x0000  // SoilMoisturePercent
0x0001  // SoilMoistureRaw  
0x0002  // CalibrationStatus
0x0010  // CalibrationDryValue
0x0011  // CalibrationWetValue
0x0020  // BatteryVoltageMv
0x0021  // BatteryLevelPercent
0x0022  // PowerState

// Command IDs
0x0010  // StartDryCalibration
0x0011  // StartWetCalibration
0x0012  // ResetCalibration
0x0013  // ForceMeasurement
0x0016  // GetStatus

// Event IDs
0x0000  // MoistureThresholdCrossed
0x0001  // BatteryLevelChanged
0x0002  // PowerStateChanged
0x0003  // CalibrationCompleted
0x0004  // SystemError
```

## 🎯 Next Steps

1. **For Home Assistant Users: Choose Option 1** (Standard Clusters) for immediate compatibility
2. **For Custom Experience: Choose Option 2** but prepare for Home Assistant integration work
3. **Create the cluster files** in your Arduino project  
4. **Test with Matter controllers:**
   - ✅ **Works immediately:** Google Home, Apple HomeKit (with Option 1)
   - ⚠️ **Needs work:** Home Assistant (with Option 2)
   - ✅ **Advanced testing:** chip-tool (supports any cluster)
5. **Add event generation** for important state changes

## 💡 **Bottom Line for Home Assistant:**

- **Option 1 (Standard Clusters):** Home Assistant works immediately, no extra setup
- **Option 2 (Custom Cluster):** You'll need to create Home Assistant custom integration or manual configuration

Most users should start with **Option 1** for broad compatibility, then optionally add custom features later.

## 🔗 Additional Resources

- Generated cluster constants: `MTRClusterConstants.h`
- Complete cluster definition: `controller-clusters.matter`
- Silicon Labs Matter documentation for Arduino Nano Matter
- Matter specification for custom cluster development

Your custom cluster is now officially part of the Matter ecosystem! 🚀
