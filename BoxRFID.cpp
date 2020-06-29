#include "BoxRFID.h"

void BoxRFID::begin() {
    Log.info("Initialize RFID...");
    setInterval(250);

    pinMode(16, OUTPUT);
    SPI.begin();

    sendCommand(DIRECT_COMMANDS::SOFT_INIT);
    sendCommand(DIRECT_COMMANDS::IDLING);
    delay(1);
    sendCommand(DIRECT_COMMANDS::RESET_FIFO);

    Log.info("...initialized");
}
void BoxRFID::loop() {  
}

void BoxRFID::setSlaveSelect(bool enabled) {
    digitalWrite(16, enabled);
}

void BoxRFID::spiEnable() {
  setSlaveSelect(false);
}
void BoxRFID::spiDisable() {
  setSlaveSelect(true);
}

uint8_t BoxRFID::readRegister(uint8_t regi) {
  uint8_t data = regi & 0b00011111;
  data |= (uint8_t)REG_CMD_WORD_BITS::REGISTER_B7 | (uint8_t)REG_CMD_WORD_BITS::READ_B6;

  uint8_t res1, res2;
  spiEnable();
  SPI.setDataMode(SPI_SUB_MODE_0);
  res1 = SPI.transfer(data);
  SPI.setDataMode(SPI_SUB_MODE_1);
  res2 = SPI.transfer(0x00);
  SPI.setDataMode(SPI_SUB_MODE_0);
  spiDisable();

  Log.info("Read register %i, data=%i, res1=%i, res2=%i", regi, data, res1, res2);

  return res2;
}
bool BoxRFID::writeRegister(uint8_t regi, uint8_t value) {
  uint8_t data = regi & 0b00011111;
  data |= (uint8_t)REG_CMD_WORD_BITS::REGISTER_B7 | (uint8_t)REG_CMD_WORD_BITS::WRITE_B6;
  
  uint8_t res1, res2;
  spiEnable();
  SPI.setDataMode(SPI_SUB_MODE_0);
  res1 = SPI.transfer(data);
  SPI.setDataMode(SPI_SUB_MODE_1);
  res2 = SPI.transfer(value);
  SPI.setDataMode(SPI_SUB_MODE_0);
  spiDisable();

  Log.info("Write register %i, data=%i, value=%i, res1=%i, res2=%i", regi, data, value, res1, res2);
  return true; //TODO;
}
bool BoxRFID::sendCommand(DIRECT_COMMANDS command) {
  return sendCommand((uint8_t)command);
}
bool BoxRFID::sendCommand(uint8_t command) {
  uint8_t data = command & 0b00011111;
  data |= (uint8_t)REG_CMD_WORD_BITS::COMMAND_B7 | (uint8_t)REG_CMD_WORD_BITS::WRITE_B6;

  uint8_t res1, res2;
  spiEnable();
  SPI.setDataMode(SPI_SUB_MODE_0);
  res1 = SPI.transfer(data);
  SPI.setDataMode(SPI_SUB_MODE_1);
  res2 = SPI.transfer(0x00); //Dummy transfer, see TRF796xA SPI Design Tips (sloa140)
  SPI.setDataMode(SPI_SUB_MODE_0);
  spiDisable();

  Log.info("Write command %i, data=%i, res1=%i, res2=%i", command, data, res1, res2);
  return true; //TODO;
}