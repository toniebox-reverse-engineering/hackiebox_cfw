#include "BoxLEDs.h"

void BoxLEDs::begin() {
    //pinMode(19, OUTPUT); //Red Analog Pin TODO
    pinMode(21, OUTPUT); //Green
    pinMode(17, OUTPUT); //Blue
}

void BoxLEDs::loop() {
    
}

void BoxLEDs::setRed(bool enabled) {
    //digitalWrite(19, enabled);
}
void BoxLEDs::setGreen(bool enabled) {
    digitalWrite(21, enabled);
}
void BoxLEDs::setBlue(bool enabled) {
    digitalWrite(17, enabled);
}

bool BoxLEDs::getRed() {
    //return digitalRead(19);
}
bool BoxLEDs::getGreen() {
    return digitalRead(21);
}
bool BoxLEDs::getBlue() {
    return digitalRead(17);
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