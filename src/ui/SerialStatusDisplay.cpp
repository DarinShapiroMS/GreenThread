#include "SerialStatusDisplay.h"
#include <Arduino.h>

void SerialStatusDisplay::begin() {
  Serial.begin(115200);
  Serial.println("SerialStatusDisplay initialized");
}

void SerialStatusDisplay::handleEvent(StatusEvent event) {
  Serial.print("[Event] ");
  Serial.println(static_cast<int>(event));
}

void SerialStatusDisplay::showMoisture(float percent) {
  Serial.printf("[Moisture] %.1f%%\n", percent);
}

void SerialStatusDisplay::showMessage(const char* msg) {
  Serial.printf("[Message] %s\n", msg);
}

void SerialStatusDisplay::update() {
  // No-op
}
