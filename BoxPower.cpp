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
    setInterval(5000);

    Log.info("Initialize BoxPower class, sleepMinutes=%i", _sleepMinutes);
}

void BoxPower::loop() {
    uint32_t millis_tmp = millis();
    uint32_t deltaMinutes = (millis_tmp - _lastFeed) / (1000 * 60);
    if (_sleepMinutes > 0 && deltaMinutes >= _sleepMinutes) {
        Log.verbose("millis_tmp=%l, _lastFeed=%l, deltaMinutes=%l", millis_tmp, _lastFeed, deltaMinutes);
        Events.handlePowerEvent(PowerEvent::BOX_IDLE);
    }
}

void BoxPower::feedSleepTimer() {
    _lastFeed = millis();
    Log.verbose("Sleep timer reset, _lastFeed=%l, freeMEM=%i", _lastFeed, freeMemory());
}

void BoxPower::_preparePowerDown() {
    Log.info("Prepare power down...");
    //TODO
    //smartconfig down
    //disable watchdog
    setSdPower(false);
    setOtherPower(false);
    Box.boxLEDs.setAllBool(false);
}
void BoxPower::reset() {
    Events.handlePowerEvent(PowerEvent::PRE_RESET);

    _preparePowerDown();
    PRCMMCUReset(true);
}
void BoxPower::hibernate() {
    Events.handlePowerEvent(PowerEvent::PRE_HIBERNATE);

    _preparePowerDown();
    //enable ear wakeup interrupt
    PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2 | PRCM_HIB_GPIO4);
    //TODO
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