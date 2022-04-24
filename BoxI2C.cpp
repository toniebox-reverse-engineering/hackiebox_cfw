#include "BoxI2C.h"

void BoxI2C::begin() {
    //Wire.begin(); //TODO!
}

bool BoxI2C::send_raw(uint8_t data) {
    if (!Wire.write(data)) {
        Log.error("Couldn't write into I2C Buffer");
        return false;
    }
    return true;
}
bool BoxI2C::send(uint8_t address, uint8_t target_register, uint8_t data) {
    Wire.beginTransmission(address);
    if (!send_raw(target_register)) return false;
    if (!send_raw(data)) return false;
    
    uint8_t result = Wire.endTransmission(false);
    if (!result) return true;
    Log.error("Couldn't send I2C buffer, error=%i", result);
    return false;
}

uint8_t BoxI2C::readByte(uint8_t address, uint8_t source_register) {
    Wire.beginTransmission(0x18);
    if (!send_raw(source_register)) return false;
    Wire.endTransmission(false);
    if (!Wire.requestFrom(0x18 ,1)) return false;
    int result = Wire.read();
    //Log.info("readI2C addr=%i reg=%i result=%i", address, source_register, result);
    if (result == -1) return false;
    return (uint8_t)result;
}