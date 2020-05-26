#include "BoxBattery.h"

#include "Hackiebox.h"

void BoxBattery::begin() {
    reloadConfig();

    pinMode(8, INPUT); //Charger pin

    logBatteryStatus();
}
void BoxBattery::loop() {
    if (!isChargerConnected()) {
        if (isBatteryCritical()) {
            Log.info("Battery is critical, please connect the charger, hibernating!");
            logBatteryStatus();
            Box.boxPower.hibernate();
        } else if (isBatteryLow()) {
            Log.info("Battery is low, please connect the charger!");
            logBatteryStatus();
        }
    }
}

bool BoxBattery::isChargerConnected() {
    return digitalRead(8);
}
uint16_t BoxBattery::getBatteryAdcRaw() {
    return analogRead(60);
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