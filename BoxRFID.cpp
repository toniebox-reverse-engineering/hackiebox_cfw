#include "BoxRFID.h"

void BoxRFID::begin() {
    Log.info("Initialize RFID...");
    setInterval(250);

    SPI.begin();
    sendCommand(0xF0);

    Log.info("...initialized");
}
void BoxRFID::loop() {  
}

void BoxRFID::setSlaveSelect(bool enabled) {
    digitalWrite(16, enabled);
}

void BoxRFID::sendCommand(int address, int value) {
  uint8_t res_address, res_value;
  setSlaveSelect(false);
  //  send in the address and value via SPI:
  res_address = SPI.transfer(address);
  res_value = SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  setSlaveSelect(true);

  Log.info("SPI address %X, answer %X, value %X, answer %X", address, res_address, value, res_value);
}
void BoxRFID::sendCommand(int value) {
  sendCommand(value, 0x00);
}