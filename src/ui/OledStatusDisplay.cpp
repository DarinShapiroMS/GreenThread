#include "OledStatusDisplay.h"
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

void OledStatusDisplay::begin() {
  display.begin();
  display.setFont(u8g2_font_6x10_tf);
  drawCentered("OLED initialized");
}

void OledStatusDisplay::handleEvent(StatusEvent event) {
  switch (event) {
    case StatusEvent::BootStarting:
    case StatusEvent::BootSensorInit:
    case StatusEvent::BootNetworkInit:
    case StatusEvent::BootMatterInit:
      drawCentered("Booting...");
      break;
    case StatusEvent::BootComplete:
      drawCentered("Boot complete");
      break;
    case StatusEvent::ThreadConnected:
      drawCentered("Thread connected");
      break;
    case StatusEvent::ThreadDisconnected:
      drawCentered("Thread disconnected");
      break;
    case StatusEvent::MatterOnline:
      drawCentered("Matter online");
      break;
    case StatusEvent::MatterOffline:
      drawCentered("Matter offline");
      break;
    case StatusEvent::BatteryLow:
      drawCentered("Battery low");
      break;
    case StatusEvent::Error:
      drawCentered("Error occurred");
      break;
    case StatusEvent::MoisturePublished:
      drawCentered("Moisture published");
      break;
    default:
      break;
  }
}

void OledStatusDisplay::showMoisture(float percent) {
  display.clearBuffer();
  
  // Title
  display.setCursor(0, 12);
  display.print("Soil Moisture");
  
  // Large percentage display
  display.setFont(u8g2_font_10x20_tf);  // Larger font for percentage
  display.setCursor(0, 35);
  display.print(percent, 1);
  display.print("%");
  
  // Progress bar
  int barWidth = 100;
  int barHeight = 8;
  int barX = 14;
  int barY = 45;
  
  // Draw bar outline
  display.drawFrame(barX, barY, barWidth, barHeight);
  
  // Fill bar based on percentage
  int fillWidth = (percent / 100.0) * (barWidth - 2);
  if (fillWidth > 0) {
    display.drawBox(barX + 1, barY + 1, fillWidth, barHeight - 2);
  }
  
  // Restore normal font
  display.setFont(u8g2_font_6x10_tf);
  
  display.sendBuffer();
}

void OledStatusDisplay::showMessage(const char* msg) {
  drawCentered(msg);
}

void OledStatusDisplay::update() {
  // No periodic update logic required
}

void OledStatusDisplay::showBattery(float voltage, bool isLow) {
  // Add battery icon in top-right corner of current display
  drawBatteryIcon(110, 2, voltage, isLow);
}

void OledStatusDisplay::drawBatteryIcon(int x, int y, float voltage, bool isLow) {
  // Draw battery outline (12x6 pixels)
  display.drawFrame(x, y, 12, 6);
  display.drawBox(x + 12, y + 1, 2, 4);  // Battery terminal
  
  // Calculate fill level (assuming 3.0V-4.2V range)
  float fillPercent = constrain((voltage - 3.0) / 1.2, 0.0, 1.0);
  int fillWidth = fillPercent * 10;
  
  if (isLow) {
    // Blink effect for low battery (simple implementation)
    static unsigned long lastBlink = 0;
    static bool blinkState = false;
    if (millis() - lastBlink > 500) {
      blinkState = !blinkState;
      lastBlink = millis();
    }
    if (blinkState && fillWidth > 0) {
      display.drawBox(x + 1, y + 1, fillWidth, 4);
    }
  } else if (fillWidth > 0) {
    display.drawBox(x + 1, y + 1, fillWidth, 4);
  }
}

void OledStatusDisplay::drawCentered(const char* msg) {
  display.clearBuffer();
  int16_t x = (128 - strlen(msg) * 6) / 2;
  int16_t y = 32;
  display.setCursor(x, y);
  display.print(msg);
  display.sendBuffer();
}
