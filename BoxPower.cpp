#include "BoxPower.h"

#include "Hackiebox.h"
#include <driverlib/prcm.h>

void BoxPower::initPins() {
    pinMode(58, OUTPUT); //SD Power pin
    pinMode(61, OUTPUT); //Audio, Accelerometer, RFID, LED Blue / Red?
}

void BoxPower::begin() {
    _sleepMinutes = Config.get()->battery.sleepMinutes;
    _lastFeed = millis();

    Log.info("Initialize BoxPower class, sleepMinutes=%i", _sleepMinutes);
}

void BoxPower::loop() {
    uint32_t millis_tmp = millis();
    uint32_t deltaMinutes = (millis_tmp - _lastFeed) / (1000 * 60);
    if (_sleepMinutes > 0 && deltaMinutes >= _sleepMinutes) {
        Log.verbose("millis_tmp=%l, _lastFeed=%l, deltaMinutes=%l", millis_tmp, _lastFeed, deltaMinutes);
        Log.info("Box not used, powering off.");
        hibernate();
    }
}

void BoxPower::feedSleepTimer() {
    _lastFeed = millis();
    Log.verbose("Sleep timer reset, _lastFeed=%l", _lastFeed);
}

void BoxPower::_preparePowerDown() {
    Log.info("Prepare power down...");
    //TODO
    //smartconfig down
    //disable watchdog
    setSdPower(false);
    setOtherPower(false);
    Box.boxLEDs.setAll(false);
}
void BoxPower::reset() {
    _preparePowerDown();
    Log.info("Reset box");
    PRCMMCUReset(true);
}
void BoxPower::hibernate() {
    _preparePowerDown();
    Log.info("Go into hibernation");

    //TODO
    //enable ear wakeup interrupt
    PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2 | PRCM_HIB_GPIO4);
    //Utils_SpiFlashDeepPowerDown();
    PRCMHibernateEnter();
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