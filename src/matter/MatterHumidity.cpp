#include "MatterHumidity.h"
#include "../hardware/PowerManager.h"

uint32_t MatterHumidity::getNormalSleepInterval() const {
    return powerManager ? powerManager->getNormalSleepInterval() : 0;
}

void MatterHumidity::setNormalSleepInterval(uint32_t interval) {
    if (powerManager) {
        powerManager->setNormalSleepInterval(interval);
    }
}

uint32_t MatterHumidity::getExtendedSleepInterval() const {
    return powerManager ? powerManager->getExtendedSleepInterval() : 0;
}

void MatterHumidity::setExtendedSleepInterval(uint32_t interval) {
    if (powerManager) {
        powerManager->setExtendedSleepInterval(interval);
    }
}

uint32_t MatterHumidity::getLowPowerSleepInterval() const {
    return powerManager ? powerManager->getLowPowerSleepInterval() : 0;
}

void MatterHumidity::setLowPowerSleepInterval(uint32_t interval) {
    if (powerManager) {
        powerManager->setLowPowerSleepInterval(interval);
    }
}

uint32_t MatterHumidity::getUsbSleepInterval() const {
    return powerManager ? powerManager->getUsbSleepInterval() : 0;
}

void MatterHumidity::setUsbSleepInterval(uint32_t interval) {
    if (powerManager) {
        powerManager->setUsbSleepInterval(interval);
    }
}

float MatterHumidity::getBatteryNormalThresh() const {
    return powerManager ? powerManager->getBatteryNormalThresh() : 0.0;
}

void MatterHumidity::setBatteryNormalThresh(float thresh) {
    if (powerManager) {
        powerManager->setBatteryNormalThresh(thresh);
    }
}

float MatterHumidity::getBatteryExtendedThresh() const {
    return powerManager ? powerManager->getBatteryExtendedThresh() : 0.0;
}

void MatterHumidity::setBatteryExtendedThresh(float thresh) {
    if (powerManager) {
        powerManager->setBatteryExtendedThresh(thresh);
    }
}

float MatterHumidity::getBatteryCriticalThresh() const {
    return powerManager ? powerManager->getBatteryCriticalThresh() : 0.0;
}

void MatterHumidity::setBatteryCriticalThresh(float thresh) {
    if (powerManager) {
        powerManager->setBatteryCriticalThresh(thresh);
    }
}

uint8_t MatterHumidity::getPowerState() const {
    return powerManager ? static_cast<uint8_t>(powerManager->getCurrentState()) : 0;
}

uint32_t MatterHumidity::getCurrentSleepInterval() const {
    return powerManager ? powerManager->getCurrentSleepInterval() : 0;
}
