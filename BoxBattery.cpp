#include "BoxBattery.h"
#include "BoxEvents.h"

void BoxBattery::begin() {
    reloadConfig();

    pinMode(8, INPUT); //Charger pin

    _wasLow = false;
    _wasCritical = false;
    _batteryAdcRaw = analogRead(60);
    _batteryAdcLowRaw = 9999;

    loop();
    logBatteryStatus();
}
void BoxBattery::loop() {
    _batteryAdcRaw = analogRead(60);
    _charger.read();
    
    if (_batteryAdcRaw < _batteryAdcLowRaw || isChargerConnected())
        _batteryAdcLowRaw = _batteryAdcRaw;

    if (_charger.wasPressed()) {
        Events.handleBatteryEvent(BatteryEvent::CHR_CONNECT);
    } else if (_charger.wasReleased()) {
        Events.handleBatteryEvent(BatteryEvent::CHR_DISCONNECT);
    }

    if (!isChargerConnected()) {
        if (!_wasCritical && isBatteryCritical()) {
            _wasCritical = true;
            Events.handleBatteryEvent(BatteryEvent::BAT_CRITICAL);
        } else if (!_wasLow && isBatteryLow()) {
            _wasLow = true;
            Events.handleBatteryEvent(BatteryEvent::BAT_LOW);
        }
    } else {
        _wasLow = false;
        _wasCritical = false;
    }
}

bool BoxBattery::isChargerConnected() {
    if (_charger.isPressed())
        return true;
    return false;
}
uint16_t BoxBattery::getBatteryAdcRaw() {
    return _batteryAdcRaw;
}
uint16_t BoxBattery::getBatteryVoltage() {
    if (isChargerConnected()) {
        return 10000 * getBatteryAdcRaw() / _batteryVoltageChargerFactor;
    }
    return 10000 * getBatteryAdcRaw() / _batteryVoltageFactor;
}
bool BoxBattery::isBatteryLow() {
    if (getBatteryAdcRaw() < _batteryLowAdc)
        return true;
    return false;
}
bool BoxBattery::isBatteryCritical() {
    if (getBatteryAdcRaw() < _batteryCriticalAdc)
        return true;
    return false;
}

void BoxBattery::logBatteryStatus() {
    int voltageDec = getBatteryVoltage();
    int voltageNum = voltageDec / 100;
    voltageDec = voltageDec - voltageNum * 100;

    Log.info("Battery Status:");
    Log.info(" Charging: %T", isChargerConnected());
    Log.info(" ADC Raw: %c", getBatteryAdcRaw());
    Log.info(" Estimated Voltage: %d.%dV", voltageNum, voltageDec);
    Log.info(" Battery Low: %T", isBatteryLow());
    Log.info(" Battery Critical: %T", isBatteryCritical());
}

void BoxBattery::reloadConfig() { 
    ConfigStruct* config = Config.get();

    _batteryVoltageFactor = config->battery.voltageFactor;
    _batteryVoltageChargerFactor = config->battery.voltageChargerFactor;
    _batteryLowAdc = config->battery.lowAdc;
    _batteryCriticalAdc = config->battery.criticalAdc;
}