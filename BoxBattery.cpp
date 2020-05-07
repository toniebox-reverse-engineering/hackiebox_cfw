#include "BoxBattery.h"

void BoxBattery::begin() {
    pinMode(8, INPUT); //Charger pin

    logBatteryStatus();
}
void BoxBattery::loop() {
    if (isBatteryLow() && !isChargerConnected()) {
        //Powerdown?
        Log.info("Battery is low, please connect the charger!");
        logBatteryStatus();
    }
}

bool BoxBattery::isChargerConnected() {
    return digitalRead(8);
}
int BoxBattery::getBatteryAdcRaw() {
    return analogRead(60);
}
int BoxBattery::getBatteryVoltage() {
    if (isChargerConnected()) {
        return 10000 * getBatteryAdcRaw() / _batteryVoltageChargerFactor;
    }
    return 10000 * getBatteryAdcRaw() / _batteryVoltageFactor;

}
bool BoxBattery::isBatteryLow() {
    if (getBatteryAdcRaw() < _batteryMinimalAdc)
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
}