#include "BoxRFID.h"
#include "Hackiebox.h"

void rfid_irq() {
  Box.boxRFID.receivedInterrupt();
}

void BoxRFID::receivedInterrupt() {
  IRQ_STATUS irqStatus;
  interrupt = true;
  do {
    irqStatus = (IRQ_STATUS)readIrqRegister();

    if (irqStatus == (IRQ_STATUS)((uint8_t)IRQ_STATUS::TX_COMPLETE | (uint8_t)IRQ_STATUS::FIFO_HIGH_OR_LOW)) { // TX active and only 3 bytes left in FIFO
      trfStatus = TRF_STATUS::TX_WAIT; 
      break;
    } else {
      processInterrupt(irqStatus);
    }
  } while (digitalRead(IRQ_PIN)); 
}
void BoxRFID::processInterrupt(IRQ_STATUS irqStatus) {
  if (irqStatus == (IRQ_STATUS)((uint8_t)IRQ_STATUS::TX_COMPLETE | (uint8_t)IRQ_STATUS::FIFO_HIGH_OR_LOW)) {
    trfStatus = TRF_STATUS::TX_COMPLETE; 
  } else if (irqStatus == IRQ_STATUS::TX_COMPLETE) {
    trfStatus = TRF_STATUS::TX_COMPLETE;
    sendCommand(DIRECT_COMMANDS::RESET_FIFO);
  } else if((IRQ_STATUS)((uint8_t)irqStatus & (uint8_t)IRQ_STATUS::COLLISION_ERROR) == IRQ_STATUS::COLLISION_ERROR) {
    resetRFID();
    readIrqRegister();
    trfStatus = TRF_STATUS::COLLISION_ERROR;
    Log.error("COLLISION_ERROR not handled IRQ_STATUS=%X", irqStatus); //TODO
  } else if (irqStatus == (IRQ_STATUS)((uint8_t)IRQ_STATUS::RX_COMPLETE | (uint8_t)IRQ_STATUS::FIFO_HIGH_OR_LOW)) { 
    trfStatus = TRF_STATUS::RX_WAIT;
    trfRxLength = readRegister(REGISTER::FIFO_STATUS);
    trfRxLength = (0x0F & trfRxLength) + 1;
    if (FIFO_SIZE > (trfOffset+trfRxLength)) {
      readRegisterCont(REGISTER::FIFO, &trfBuffer[trfOffset], trfRxLength);
      trfOffset += trfRxLength;
    } else {
        trfStatus == TRF_STATUS::PROTOCOL_ERROR;
        Log.error("Read buffer to small, size=%i, count=%", FIFO_SIZE, (trfOffset+trfRxLength));
        return;
    }
    trfStatus = TRF_STATUS::RX_WAIT_EXTENSION;
    trfRxLength = trfOffset;
  } else if (irqStatus == IRQ_STATUS::RX_COMPLETE) {
    trfRxLength = readRegister(REGISTER::FIFO_STATUS);
    trfRxLength = (0x0F & trfRxLength) + 1;
    readRegisterCont(REGISTER::FIFO, &trfBuffer[trfOffset], trfRxLength);
    trfOffset += trfRxLength;
    sendCommand(DIRECT_COMMANDS::RESET_FIFO);
    if (trfStatus == TRF_STATUS::RX_WAIT_EXTENSION)
      trfRxLength = trfOffset;
    trfStatus = TRF_STATUS::RX_COMPLETE;
  } else if (irqStatus == (IRQ_STATUS)((uint8_t)IRQ_STATUS::RX_COMPLETE | (uint8_t)IRQ_STATUS::NO_RESPONSE)) { 
    trfStatus = TRF_STATUS::RX_WAIT_EXTENSION;
  } else if((IRQ_STATUS)((uint8_t)irqStatus & (uint8_t)IRQ_STATUS::CRC_ERROR) == IRQ_STATUS::CRC_ERROR) {
    resetRFID();
    trfStatus = TRF_STATUS::PROTOCOL_ERROR;
    Log.error("CRC_ERROR not handled IRQ_STATUS=%X", irqStatus); //TODO
  } else if((IRQ_STATUS)((uint8_t)irqStatus & (uint8_t)IRQ_STATUS::FRAMING_ERROR) == IRQ_STATUS::FRAMING_ERROR) {
    if ((IRQ_STATUS)((uint8_t)irqStatus & (uint8_t)IRQ_STATUS::FIFO_HIGH_OR_LOW) == IRQ_STATUS::FIFO_HIGH_OR_LOW) {
      trfStatus = TRF_STATUS::RX_WAIT;
    } else {
      resetRFID();
      trfStatus = TRF_STATUS::PROTOCOL_ERROR;
      Log.error("FRAMING_ERROR not handled IRQ_STATUS=%X", irqStatus); //TODO
    }
  } else if (irqStatus == IRQ_STATUS::IDLING) {
    trfStatus = TRF_STATUS::NO_RESPONSE_RECEIVED;
  } else if (irqStatus == IRQ_STATUS::NO_RESPONSE) {
    trfStatus = TRF_STATUS::NO_RESPONSE_RECEIVED_15693;
    trfOffset = 0;
  } else {
    trfStatus = TRF_STATUS::PROTOCOL_ERROR;
    Log.error("Unknown IRQ_STATUS=%X", irqStatus);
    //TODO
  }
}

void BoxRFID::clearInterrupt() {
  interrupt = false;
}
bool BoxRFID::readInterrupt() {
  return interrupt;
}

void BoxRFID::begin() {
    Log.info("Initialize RFID...");
    setInterval(2000);

    pinMode(16, OUTPUT);
    pinMode(IRQ_PIN, INPUT);
    attachInterrupt(IRQ_PIN, rfid_irq, RISING);
    SPI.begin();
    SPI.setDataMode(SPI_SUB_MODE_0);

    resetRFID();

    Log.info("...initialized");
}
void BoxRFID::loop() {  
    resetRFID();

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
  uint8_t data = *buffer & 0b00011111;
  data |= (uint8_t)REG_CMD_WORD_BITS::REGISTER_B7 | (uint8_t)REG_CMD_WORD_BITS::READ_B6 | (uint8_t)REG_CMD_WORD_BITS::CONTINUOUS_MODE_REG_B5;

  spiEnable();
  SPI.transfer(data);
  SPI.setDataMode(SPI_SUB_MODE_1);

  while(length-- > 0) {
    *buffer = SPI.transfer(0x00);
    buffer++;
    //Log.info(" length=%i, result=%X", length, *buffer);
  }
  SPI.setDataMode(SPI_SUB_MODE_0);
  spiDisable();
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

void BoxRFID::sendRaw(uint8_t* buffer, uint8_t length) {
  if (FIFO_SIZE+5 > length) {
    sendRawSPI(buffer, length, false);
  } else {
    Log.error("Not implemented long FIFO writes");
  }
}
void BoxRFID::sendRawSPI(uint8_t* buffer, uint8_t length, bool continuedSend) {
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
  uint8_t g_ui8TagDetectedCount;
  uint8_t ui8LoopCount = 0;
  uint8_t ui8Offset = 0;
  
	trfBuffer[ui8Offset++] = 0x8F;		// Reset FIFO
	trfBuffer[ui8Offset++] = 0x91;		// Send  //CRC 0x91 Non CRC CRC 0x90
	trfBuffer[ui8Offset++] = 0x3D;		// Write Continuous
	trfBuffer[ui8Offset++] = 0x00;		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	trfBuffer[ui8Offset++] = 0x30;		// Length of packet in bytes - lower and broken nibbles of transmit byte length
	trfBuffer[ui8Offset++] = 0x26;		// ISO15693 flags
	trfBuffer[ui8Offset++] = 0x01;		// Inventory command code
	trfBuffer[ui8Offset++] = 0x00;		// Mask Length = 0 (Also not sending AFI)
	/*trfBuffer[ui8Offset++] = 0x00;		//
	trfBuffer[ui8Offset++] = 0x00;*/		//

  readIrqRegister();
  sendCommand(DIRECT_COMMANDS::RESET_FIFO);
  sendRaw(&trfBuffer[0], ui8Offset); //TODO TRF79xxA_writeRaw
  trfStatus = waitRxData(15, 5);

	if (trfStatus == TRF_STATUS::RX_COMPLETE) { // If data has been received
		if (trfBuffer[0] == 0x00)	{	// Confirm "no error" in response flags byte
      if (trfRxLength == 14) {
        uint8_t g_pui8Iso15693UId[8];
        // UID Starts at the 3rd received bit (1st is flags and 2nd is DSFID)
        for (ui8LoopCount = 6; ui8LoopCount < 14; ui8LoopCount++) {
          g_pui8Iso15693UId[ui8LoopCount-6] = trfBuffer[ui8LoopCount];	// Store UID into a Buffer
        }

        Log.info("RFID UID: ");
        Log.print(" ");
        for (ui8LoopCount = 0; ui8LoopCount < 8; ui8LoopCount++) {
          Log.printf("%x ", g_pui8Iso15693UId[7-ui8LoopCount]);		// Send UID to host
        }
        Log.println();
        g_ui8TagDetectedCount = 1;
        return true;
      } else {
        Log.error("Received invalid answer. Length should be %i but is %i", 12, trfRxLength);
      }
		} else {
      Log.error("Error flag=%X while reading", trfStatus);
    }
	} else {
    Log.error("Unexpected TRF_STATUS for inventory %X", trfStatus);
	}

	return false;
}

uint8_t BoxRFID::readIrqRegister() {
  uint8_t buffer[2];
  buffer[0] = (uint8_t)REGISTER::IRQ_STATUS;
  buffer[1] = (uint8_t)REGISTER::IRQ_MASK;
  readRegisterCont(buffer, 2);
  Log.info("IRQ_STATUS=%X", buffer[0]);
  return buffer[0];
}

BoxRFID::TRF_STATUS BoxRFID::waitRxData(uint8_t txTimeout, uint8_t rxTimeout) {
  switch (trfStatus) {
  case TRF_STATUS::TRF_IDLE:
  case TRF_STATUS::TX_WAIT:
    waitTxIRQ(txTimeout);
    waitRxIRQ(rxTimeout);
    break;
  case TRF_STATUS::TX_COMPLETE:
    waitRxIRQ(rxTimeout);
    break;
  case TRF_STATUS::NO_RESPONSE_RECEIVED_15693:
    break;
  case TRF_STATUS::COLLISION_ERROR:
    //TODO Firmware example has code that does nothing here
    break;
  case TRF_STATUS::RX_COMPLETE:
  case TRF_STATUS::RX_WAIT:
  case TRF_STATUS::RX_WAIT_EXTENSION:
    break;
  
  default:
    trfStatus = TRF_STATUS::TX_ERROR;
    break;
  }
  return trfStatus;
}

void BoxRFID::waitTxIRQ(uint8_t txTimeout) {
  trfStatus = TRF_STATUS::RX_WAIT;
  while (trfStatus != TRF_STATUS::TX_COMPLETE && trfStatus != TRF_STATUS::TX_ERROR) {
    clearInterrupt();
    BoxTimer timer;
    timer.setTimer(txTimeout);
    while (!readInterrupt() && timer.isRunning()) {
      timer.tick();
    }
    if (!timer.isRunning())
      Log.error("waitTxIRQ Timeout");
    if (trfStatus != TRF_STATUS::TX_COMPLETE) {
      if(trfStatus == TRF_STATUS::TX_WAIT) {
        waitTxIRQ(txTimeout);
      } else {
        trfStatus = TRF_STATUS::TX_ERROR;
      }
    }
  }
}
void BoxRFID::waitRxIRQ(uint8_t rxTimeout) {
  trfOffset = 0;
  trfStatus = TRF_STATUS::RX_WAIT;
  while (trfStatus == TRF_STATUS::RX_WAIT) {
    clearInterrupt();
    BoxTimer timer;
    timer.setTimer(rxTimeout);
    while (!readInterrupt() && timer.isRunning()) {
      timer.tick();
    }
    if (!timer.isRunning())
      Log.error("waitRxIRQ Timeout");
    while (trfStatus == TRF_STATUS::RX_WAIT_EXTENSION) {
      clearInterrupt();
      timer.setTimer(5); //from firmware example
      while (!readInterrupt() && timer.isRunning()) {
        timer.tick();
        }
      if (!timer.isRunning())
        Log.error("waitRxIRQ Timeout 2");
      if (trfStatus == TRF_STATUS::NO_RESPONSE_RECEIVED)
        trfStatus = TRF_STATUS::RX_COMPLETE;
    }
    if (trfStatus == TRF_STATUS::RX_WAIT)
      trfStatus = TRF_STATUS::NO_RESPONSE_RECEIVED;
  }  
}

void BoxRFID::resetRFID() {
  sendCommand(DIRECT_COMMANDS::SOFT_INIT);
  sendCommand(DIRECT_COMMANDS::IDLING);
  delay(1);
  clearInterrupt();
  sendCommand(DIRECT_COMMANDS::RESET_FIFO);
  trfOffset = 0;
  trfRxLength = 0;
  trfStatus = TRF_STATUS::TRF_IDLE;
}