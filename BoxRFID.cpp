#include "BoxRFID.h"
#include "Hackiebox.h"

void rfid_irq() {
  Box.boxRFID.receivedInterrupt();
}

void BoxRFID::receivedInterrupt() {
  interrupt = true;
}
void BoxRFID::clearInterrupt(bool clearRegister) {
  interrupt = false;
  if (clearRegister)
    clearIrqRegister();
}
bool BoxRFID::readInterrupt() {
  return interrupt;
}

void BoxRFID::begin() {
    Log.info("Initialize RFID...");
    setInterval(2000);

    pinMode(16, OUTPUT);
    pinMode(IRQ_PIN, INPUT);
    clearInterrupt(false);
    attachInterrupt(IRQ_PIN, rfid_irq, RISING);
    SPI.begin();
    SPI.setDataMode(SPI_SUB_MODE_0);

    /* RESET START */
    sendCommand(DIRECT_COMMANDS::SOFT_INIT);
    sendCommand(DIRECT_COMMANDS::IDLING);
    delay(1);
    sendCommand(DIRECT_COMMANDS::RESET_FIFO);

    Log.info("...initialized");
}
void BoxRFID::loop() {  
    /* RESET START */
    sendCommand(DIRECT_COMMANDS::SOFT_INIT);
    sendCommand(DIRECT_COMMANDS::IDLING);
    delay(1);
    sendCommand(DIRECT_COMMANDS::RESET_FIFO);
    //writeRegister(REGISTER::MODULATOR_CONTROL, 0x01); //Sys Clock Output = 13.56MHz, OOK = 100%
    //writeRegister(REGISTER::REGULATOR_CONTROL, 0x01);
    /* RESET END */

    writeRegister(REGISTER::CHIP_STATUS_CONTROL, 0b00100001); //turnRfOn();

    /* SETUP START */
    writeRegister(REGISTER::ISO_CONTROL, 0b10000010); //ISO / IEC 15693 high bit rate, 26.48 kbps, one subcarrier, 1 out of 4 no crcr
    writeRegister(REGISTER::IRQ_MASK, 0b00111110);
    writeRegister(REGISTER::MODULATOR_CONTROL, 0b00100001); //Sys Clock Output = 6.78MHz, OOK = 100%
    writeRegister(REGISTER::TX_PULSE_LENGTH_CONTROL, 0x80); 
    /*writeRegister(REGISTER::MODULATOR_CONTROL, 0x01); //Sys Clock Output = 13.56MHz, OOK = 100%
    writeRegister(REGISTER::RX_NO_RESPONSE_WAIT_TIME, 0x15); //No Response Wait Time
    writeRegister(REGISTER::RX_WAIT_TIME, 0x1F); //RX Wait Time*/
    /* SETUP END */

    //ISO15693_resetTagCount();

    // The VCD should wait at least 1 ms after it activated the
	  // powering field before sending the first request, to
	  // ensure that the VICCs are ready to receive it. (ISO15693-3)
    delay(20); //not 1 ms?!

    ISO15693_sendSingleSlotInventory();

    writeRegister(REGISTER::CHIP_STATUS_CONTROL, 0b00000001); //turnRfOff();
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

uint8_t BoxRFID::readRegister(REGISTER regi) {
  return readRegister((uint8_t)regi);
}
uint8_t BoxRFID::readRegister(uint8_t regi) {
  uint8_t data = regi & 0b00011111;
  data |= (uint8_t)REG_CMD_WORD_BITS::REGISTER_B7 | (uint8_t)REG_CMD_WORD_BITS::READ_B6;

  uint8_t res1, res2;
  spiEnable();
  res1 = SPI.transfer(data);
  SPI.setDataMode(SPI_SUB_MODE_1);
  res2 = SPI.transfer(0x00);
  SPI.setDataMode(SPI_SUB_MODE_0);
  spiDisable();

  //Log.info("Read register %i, data=%i, res1=%i, res2=%i", regi, data, res1, res2);

  return res2;
}
void BoxRFID::readRegisterCont(REGISTER regi, uint8_t* buffer, uint8_t length) {
  readRegisterCont((uint8_t)regi, buffer, length);
}
void BoxRFID::readRegisterCont(uint8_t regi, uint8_t* buffer, uint8_t length) {
  buffer[0] = regi;
  readRegisterCont(buffer, length);
}
void BoxRFID::readRegisterCont(uint8_t* buffer, uint8_t length) {
  uint8_t data = buffer[0] & 0b00011111;
  data |= (uint8_t)REG_CMD_WORD_BITS::REGISTER_B7 | (uint8_t)REG_CMD_WORD_BITS::READ_B6 | (uint8_t)REG_CMD_WORD_BITS::CONTINUOUS_MODE_REG_B5;

  spiEnable();
  SPI.transfer(*buffer);
  SPI.setDataMode(SPI_SUB_MODE_1);
	
  while(length-- > 0) {
    *buffer = SPI.transfer(0x00);
    buffer++;
  }
  SPI.setDataMode(SPI_SUB_MODE_0);
  spiDisable();

  //Log.info("Read register continued");
}
void BoxRFID::writeRegister(REGISTER regi, uint8_t value) {
  writeRegister((uint8_t)regi, value);
}
void BoxRFID::writeRegister(uint8_t regi, uint8_t value) {
  uint8_t data = regi & 0b00011111;
  data |= (uint8_t)REG_CMD_WORD_BITS::REGISTER_B7 | (uint8_t)REG_CMD_WORD_BITS::WRITE_B6;
  
  uint8_t res1, res2;
  spiEnable();
  res1 = SPI.transfer(data);
  res2 = SPI.transfer(value);
  spiDisable();

  //Log.info("Write register %i, data=%i, value=%i, res1=%i, res2=%i", regi, data, value, res1, res2);
}
void BoxRFID::sendCommand(DIRECT_COMMANDS command) {
  return sendCommand((uint8_t)command);
}
void BoxRFID::sendCommand(uint8_t command) {
  uint8_t data = command & 0b00011111;
  data |= (uint8_t)REG_CMD_WORD_BITS::COMMAND_B7 | (uint8_t)REG_CMD_WORD_BITS::WRITE_B6;

  uint8_t res1, res2;
  spiEnable();
  res1 = SPI.transfer(data);
  res2 = SPI.transfer(0x00); //Dummy transfer, see TRF796xA SPI Design Tips (sloa140)
  spiDisable();

  //Log.info("Write command %i, data=%i, res1=%i, res2=%i", command, data, res1, res2);
}

void BoxRFID::sendRaw(uint8_t* buffer, uint8_t length, bool continuedSend) {
  spiEnable();

  if (continuedSend)
    SPI.transfer(0x3F); //Doc TODO
  
  while (length-- > 0) {
    SPI.transfer(*buffer);
    buffer++;
  }

  spiDisable();
}

bool BoxRFID::ISO15693_sendSingleSlotInventory() {
  uint8_t g_pui8TrfBuffer[100]; //may reduce size;
  TRF_STATUS g_sTrfStatus = TRF_STATUS::TRF_IDLE;
  uint8_t g_ui8TagDetectedCount;

	uint8_t ui8Offset = 0;
	uint8_t ui8LoopCount = 0;
	bool ui8Status = false;
  
	g_pui8TrfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	g_pui8TrfBuffer[ui8Offset++] = 0x91;		// Send with CRC
	g_pui8TrfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x30;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	g_pui8TrfBuffer[ui8Offset++] = 0x26;		// ISO15693 flags
	g_pui8TrfBuffer[ui8Offset++] = 0x01;		// Inventory command code
	g_pui8TrfBuffer[ui8Offset++] = 0x00;		// Mask Length = 0 (Also not sending AFI)

	//TRF79xxA_writeRaw(&g_pui8TrfBuffer[0], ui8Offset);		// Issue the ISO15693 Inventory Command

  BoxTimer timer;

  clearInterrupt();
  //<12+5 bytes can directly writeRegister
  sendRaw(&g_pui8TrfBuffer[0], ui8Offset, false);

	//g_sTrfStatus = TRF79xxA_waitRxData(5,15);			// 5 millisecond TX timeout, 15 millisecond RX timeout

  IRQ_STATUS irqStatus;
  timer.setTimer(5);
  while (!readInterrupt()) {
      timer.tick();
      if (!timer.isRunning()) {
        //Log.error("TX Timeout");
        return false;
      }
  }
  irqStatus = (IRQ_STATUS)readRegister(REGISTER::IRQ_STATUS);
  clearInterrupt();

  if (irqStatus == IRQ_STATUS::TX_COMPLETE) {
    sendCommand(DIRECT_COMMANDS::RESET_FIFO);
    timer.setTimer(15);
    while (!readInterrupt()) {
        timer.tick();
        if (!timer.isRunning()) {
          //Log.error("RX Timeout");
          return false;
        }
    }
    irqStatus = (IRQ_STATUS)readRegister(REGISTER::IRQ_STATUS);
    clearInterrupt();
  }
  
  if ((irqStatus == IRQ_STATUS::RX_COMPLETE)
    || (irqStatus == (IRQ_STATUS)((uint8_t)IRQ_STATUS::TX_COMPLETE | (uint8_t)IRQ_STATUS::RX_COMPLETE))
    || (irqStatus == (IRQ_STATUS)((uint8_t)IRQ_STATUS::TX_COMPLETE | (uint8_t)IRQ_STATUS::RX_COMPLETE | (uint8_t)IRQ_STATUS::FIFO_HIGH_OR_LOW)) ) {
    Log.info("RX_COMPLETE IRQ STATUS=%X", (uint8_t)irqStatus);
    g_sTrfStatus = TRF_STATUS::RX_COMPLETE;
  } else {
    Log.error("Unknown IRQ STATUS=%X", (uint8_t)irqStatus);
  }

	if (g_sTrfStatus == TRF_STATUS::RX_COMPLETE)		// If data has been received
	{
    uint8_t len = readRegister(REGISTER::FIFO_STATUS);
    Log.info("FIFO STATUS=%i", len);
    len = (0x0F & len) + 1;
    Log.info("RX length=%i", len);
    readRegisterCont(REGISTER::FIFO, g_pui8TrfBuffer, len);
    sendCommand(DIRECT_COMMANDS::RESET_FIFO);
    
		if (g_pui8TrfBuffer[0] == 0x00)		// Confirm "no error" in response flags byte
		{
			ui8Status = true;
      uint8_t g_pui8Iso15693UId[8];
			// UID Starts at the 3rd received bit (1st is flags and 2nd is DSFID)
			for (ui8LoopCount = 2; ui8LoopCount < 10; ui8LoopCount++) {
				g_pui8Iso15693UId[ui8LoopCount-2] = g_pui8TrfBuffer[ui8LoopCount];	// Store UID into a Buffer
			}

      Log.info("RFID UID: ");
      Log.print(" ");
			for (ui8LoopCount = 0; ui8LoopCount < 8; ui8LoopCount++) {
				Log.printf("%x ", g_pui8Iso15693UId[7-ui8LoopCount]);		// Send UID to host
			}
      Log.println();
			g_ui8TagDetectedCount = 1;
		} else {
      Log.error("Error flag=%X while reading", g_pui8TrfBuffer[0]);
    }
	}
	else
	{
		ui8Status = false;
	}

	return ui8Status;
}

void BoxRFID::clearIrqRegister() {
  uint8_t buffer[2];
  buffer[0] = (uint8_t)REGISTER::IRQ_STATUS;
  readRegisterCont(buffer, 2);
  /*
  while (digitalRead(IRQ_PIN)) {
    delay(1000);
    Log.info("Waiting for IRQ PIN to clear...");
  }
  */
}