#include "BoxPower.h"

void BoxPower::begin() {
    _sleepMinutes = Config.get()->battery.sleepMinutes;
    _lastFeed = millis();

    pinMode(58, OUTPUT); //SD Power pin
    pinMode(61, OUTPUT); //Audio, Accelerometer, RFID, LED Blue / Red?
}

void BoxPower::loop() {
    uint32_t deltaMinutes = (millis() - _lastFeed) / (1000 * 60);
    if (_sleepMinutes > 0 && deltaMinutes >= _sleepMinutes) {
        Log.info("Box not used, powering off.");
        hibernate();
    }
}

void BoxPower::feedSleepTimer() {
    _lastFeed = millis();
}

void BoxPower::hibernate() {
    //TODO
    //smartconfig down
    //enable ear wakeup interrupt
    //disable watchdog
    setSdPower(false);
    setOtherPower(false);
}

void BoxPower::setSdPower(bool power) {
    digitalWrite(58, !power);
}
void BoxPower::setOtherPower(bool power) {
    digitalWrite(61, power);
}

bool BoxPower::getSdPower() {
    return !digitalRead(58);
}
bool BoxPower::getOtherPower() {
    return digitalRead(61);
}