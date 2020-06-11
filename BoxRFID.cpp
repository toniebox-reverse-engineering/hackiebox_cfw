#include "BoxRFID.h"

void BoxRFID::begin() {
    Log.info("Initialize RFID...");
    //Enable CHIP (EN)
    pinMode(62, OUTPUT);
    digitalWrite(62, HIGH);

    setInterval(250);

    SPI.begin();
    sendCommand(0x03, 0x00);

    Log.info("...initialized");
}
void BoxRFID::loop() {  
}

void BoxRFID::setSlaveSelect(bool enabled) {
    digitalWrite(16, enabled);
}

void BoxRFID::sendCommand(int address, int value) {
  setSlaveSelect(false);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  setSlaveSelect(true);
}
void BoxRFID::sendCommand(int value) {
  setSlaveSelect(false);
  //  send in the address and value via SPI:
  SPI.transfer(value);
  SPI.transfer(0x00);
  // take the SS pin high to de-select the chip:
  setSlaveSelect(true);
}