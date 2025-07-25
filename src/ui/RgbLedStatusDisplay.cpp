#include "RgbLedStatusDisplay.h"
#include <Arduino.h>

void RgbLedStatusDisplay::begin() {
  pinMode(LED_BUILTIN,    OUTPUT);  // Red
  pinMode(LED_BUILTIN_1,  OUTPUT);  // Green
  pinMode(LED_BUILTIN_2,  OUTPUT);  // Blue
  handleEvent(StatusEvent::Booting);
}

void RgbLedStatusDisplay::handleEvent(StatusEvent event) {
  switch (event) {
    case StatusEvent::Booting:
      setColor(false, true, false); break;      // Green
    case StatusEvent::MatterOffline:
    case StatusEvent::ThreadDisconnected:
    case StatusEvent::Error:
    case StatusEvent::BatteryLow:
      setColor(true, false, false); break;      // Red
    case StatusEvent::ThreadConnected:
    case StatusEvent::MatterOnline:
      setColor(false, true, false); break;      // Green
    default:
      setColor(false, false, false); break;     // Off
  }
}

void RgbLedStatusDisplay::showMoisture(float percent) {
  int blinkDelay = constrain(map(percent, 0, 100, 300, 50), 50, 300);
  for (int i = 0; i < 10; ++i) {
    digitalWrite(LED_BUILTIN_2, HIGH);  // Blue
    delay(blinkDelay);
    digitalWrite(LED_BUILTIN_2, LOW);
    delay(blinkDelay);
  }
}

void RgbLedStatusDisplay::showMessage(const char* msg) {
  // LED can’t display text, so ignore or optionally blink a pattern
}

void RgbLedStatusDisplay::update() {
  // No-op
}

void RgbLedStatusDisplay::setColor(bool r, bool g, bool b) {
  digitalWrite(LED_BUILTIN,    r);
  digitalWrite(LED_BUILTIN_1,  g);
  digitalWrite(LED_BUILTIN_2,  b);
}
