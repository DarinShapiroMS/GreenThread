# Soil Sense - Arduino Nano Matter Soil Moisture Sensor

A professional-grade soil moisture monitoring system built on Arduino Nano Matter with Thread/Matter connectivity for Home Assistant integration.

## Features

### 🌱 **Core Functionality**
- **Soil Moisture Monitoring**: Calibrated capacitive sensor with percentage readings
- **Battery Management**: Voltage monitoring with low-battery alerts and power optimization
- **Multi-Display Support**: Auto-detection of OLED, RGB LED, or Serial displays
- **Matter/Thread Integration**: Full Home Assistant compatibility

### ⚡ **Advanced Power Management**
- **Adaptive Sleep Intervals**: Battery-aware power conservation
- **USB Override**: More responsive operation when connected to USB
- **Configurable Thresholds**: Remote adjustment via Matter attributes
- **Power State Reporting**: Real-time status via Thread network

### 🔧 **Professional Features**
- **EEPROM Calibration**: Persistent sensor calibration storage
- **Factory Reset**: Easy recalibration workflow
- **Composite Display**: Simultaneous OLED + Serial output when on USB
- **Modular Architecture**: Clean separation of concerns for maintainability

### 🏠 **Home Assistant Integration**
- **Multiple Matter Endpoints**: Separate entities for all sensor data
- **Remote Configuration**: Adjust sleep intervals and power settings
- **Real-time Monitoring**: Battery level, power state, and sensor readings
- **Automation Ready**: All attributes available for HA automations

## Hardware Requirements

- **Arduino Nano Matter** (Silicon Labs MGM240S)
- **Soil Moisture Sensor** (Capacitive, connected to A0)
- **Battery Monitor** (Voltage divider on A1)
- **Optional OLED Display** (SSD1306, I2C 0x3C)
- **Built-in RGB LEDs** for status indication

## Matter Endpoints

The device exposes multiple Matter endpoints for comprehensive monitoring:

1. **Humidity Sensor** → Soil moisture percentage (0-100%)
2. **Temperature Sensors** → Battery level, sleep intervals, calibration values
3. **Switches** → Power management controls, USB status, calibration mode
4. **Status Reporting** → Current power state, display type, system status

## Getting Started

1. **Hardware Setup**: Connect soil sensor to A0, battery monitor to A1
2. **Flash Firmware**: Upload to Arduino Nano Matter
3. **Matter Commissioning**: Add to Home Assistant via QR code
4. **Calibration**: Use dry/wet calibration workflow
5. **Configuration**: Adjust power settings via HA entities

## Project Structure

```
src/
├── config/         # Configuration constants
├── hardware/       # Hardware abstraction layer
│   ├── SensorManager.cpp/h
│   ├── BatteryMonitor.cpp/h
│   ├── CalibrationManager.cpp/h
│   └── PowerManager.cpp/h
├── matter/         # Matter/Thread integration
│   └── MatterMultiSensor.cpp/h
└── ui/             # Display implementations
    ├── StatusDisplay.h
    ├── OledStatusDisplay.cpp/h
    ├── RgbLedStatusDisplay.cpp/h
    ├── SerialStatusDisplay.cpp/h
    ├── CompositeStatusDisplay.cpp/h
    └── DisplayFactory.cpp/h
```

## Development

This project uses a modular architecture with clear separation between hardware abstraction, user interface, and networking layers. The codebase is designed for easy testing, maintenance, and future expansion.

### Build System
- **Arduino CLI** with VS Code tasks
- **Silicon Labs Arduino Core** v2.3.0+
- **Matter Protocol Stack** enabled

### Version Control
- Clean commit history with feature branches
- Automated testing of builds before merge
- Tagged releases for stable versions

## License

MIT License - see LICENSE file for details

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes with tests
4. Submit a pull request

## Acknowledgments

- **Silicon Labs** for Arduino Matter support
- **Arduino Team** for the development platform
- **Home Assistant** community for Matter integration
- **Thread Group** for the mesh networking standard
