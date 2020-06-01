#include "BoxLEDs.h"
#include "wiring_private.h"

void BoxLEDs::begin() {
    PWMPrepare(19); //Red
    PWMPrepare(21); //Green
    PWMPrepare(17); //Blue
    //pinMode(21, OUTPUT); //Green
    //pinMode(17, OUTPUT); //Blue

    _stateRed = 0x01;
    _stateGreen = 0x01;
    _stateBlue = 0x01;
}

void BoxLEDs::loop() {
    
}

void BoxLEDs::setRed(bool enabled) {
    if (enabled) {
        _stateRed = 0xFE;
    } else {
        _stateRed = 0x01;
    }
    analogWrite(19, _stateRed);
}
void BoxLEDs::setGreen(bool enabled) {
    if (enabled) {
        _stateGreen = 0xFE;
    } else {
        _stateGreen = 0x01;
    }
    analogWrite(21, _stateGreen);
    //digitalWrite(21, enabled);
}
void BoxLEDs::setBlue(bool enabled) {
    if (enabled) {
        _stateBlue = 0xFE;
    } else {
        _stateBlue = 0x01;
    }
    analogWrite(17, _stateBlue);
    //digitalWrite(17, enabled);
}

bool BoxLEDs::getRed() {
    if (_stateRed > 0x01)
        return true;
    return false;
}
bool BoxLEDs::getGreen() {
    if (_stateGreen > 0x01)
        return true;
    return false;
    //return digitalRead(21);
}
bool BoxLEDs::getBlue() {
    if (_stateBlue > 0x01)
        return true;
    return false;
    //return digitalRead(17);
}

void BoxLEDs::setAll(bool enabled) {
    setRed(enabled);
    setGreen(enabled);
    setBlue(enabled);
}

void BoxLEDs::testLEDs() {
    bool ledR = getRed();
    bool ledG = getGreen();
    bool ledB = getBlue();

    Log.info("Testing LEDs...");
    delay(500);

    Log.info(" Red");
    setRed(true);
    setGreen(false);
    setBlue(false);
    delay(500);
    Log.info(" Green");
    setRed(false);
    setGreen(true);
    setBlue(false);
    delay(500);
    Log.info(" Blue");
    setRed(false);
    setGreen(false);
    setBlue(true);
    delay(500);
    Log.info(" RGB");
    setRed(true);
    setGreen(true);
    setBlue(true);

    delay(500);
    Log.info(" Off");
    setRed(false);
    setGreen(false);
    setBlue(false);

    delay(500);
    Log.info(" Reset");
    setRed(ledR);
    setGreen(ledG);
    setBlue(ledB);
    Log.info(" Test finished.");
}