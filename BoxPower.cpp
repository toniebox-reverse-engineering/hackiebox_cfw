#include "BoxPower.h"

void BoxPower::begin() {
    pinMode(58, OUTPUT); //SD Power pin
    pinMode(61, OUTPUT); //Audio, Accelerometer, RFID, LED Blue / Red?
}

void BoxPower::loop() {
    
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