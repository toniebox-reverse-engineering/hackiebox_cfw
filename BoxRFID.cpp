#include "BoxRFID.h"
#include "Hackiebox.h"
#include "BoxEvents.h"

void rfid_irq() {
  Box.boxRFID.receivedInterrupt();
}
void BoxRFID::begin() {
    Log.info("Init RFID...");
    setInterval(250);

    pinMode(16, OUTPUT);
    pinMode(IRQ_PIN, INPUT);
    attachInterrupt(IRQ_PIN, rfid_irq, RISING);
    SPI.begin();
    SPI.setDataMode(SPI_SUB_MODE_0);

    resetRFID();

    Log.info("...done");
}

void BoxRFID::loop() {  
  //TODO Hand COLLISION detection IRQ_STATUS=0x02
  resetRFID();
  initRFID();

  turnFieldOn();
  ISO15693_RESULT result;
  uint32_t knownPasswords[3] = { 0x7FFD6E5B, 0x0F0F0F0F, 0x00000000 };

  if (tagActive) {
    for (uint8_t i=0; i<3; i++) {
      result = ISO15693_getRandomSlixL(NULL);
      if (result == ISO15693_RESULT::GET_RANDOM_VALID)
        break;
    }
    
    if (result != ISO15693_RESULT::GET_RANDOM_VALID) {
      tagActive = false;
      Events.handleTagEvent(TAG_EVENT::TAG_REMOVED);
    }
  } else {
    for (uint8_t i = 0; i < 3; i++) {
      result = ISO15693_setPassSlixL(0x04, knownPasswords[i]); //reversed!
      if (result == ISO15693_RESULT::SET_PASSWORD_CORRECT) {
        Log.info("Password %X (i=%i) ok", knownPasswords[i], i);
        break;
      } else if (result == ISO15693_RESULT::SET_PASSWORD_INCORRECT) {
        Log.info("Password %X (i=%i) wrong", knownPasswords[i], i);
        turnFieldOff();
        Box.delayTask(20);
        reinitRFID();
      } else {
        break;
      }
    }

    if (result != ISO15693_RESULT::GET_RANDOM_INVALID) {
      reinitRFID();
      result = ISO15693_sendSingleSlotInventory(tagUid);
      if (result == ISO15693_RESULT::INVENTORY_VALID_RESPONSE) {
        //TODO
        tagActive = true;
        Events.handleTagEvent(TAG_EVENT::TAG_PLACED);
      } else {
        tagActive = false;
      }
    } else {
      tagActive = false;
      //Log.error("No tag? ISO15693_RESULT=%X", result);
    }
  }

  turnFieldOff();
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
    //sendCommand(DIRECT_COMMANDS::RESET_FIFO); //Really needed?
  } else if((IRQ_STATUS)((uint8_t)irqStatus & (uint8_t)IRQ_STATUS::COLLISION_ERROR) == IRQ_STATUS::COLLISION_ERROR) {
    resetRFID();
    initRFID();
    clearIrqRegister();
    trfStatus = TRF_STATUS::COLLISION_ERROR;
    Log.error("COLLISION_ERROR not handled IRQ_STATUS=%X", irqStatus); //TODO
  } else if (irqStatus == (IRQ_STATUS)((uint8_t)IRQ_STATUS::RX_COMPLETE | (uint8_t)IRQ_STATUS::FIFO_HIGH_OR_LOW)) { 
    trfStatus = TRF_STATUS::RX_WAIT;
    trfRxLength = readRegister(REGISTER::FIFO_STATUS);
    trfRxLength = (0x0F & trfRxLength) + 1;
    if (FIFO_SIZE > (trfOffset+trfRxLength)) {
        readRegisterCont(REGISTER::FIFO, &trfBuffer[trfOffset], trfRxLength);
        /*
          Log.printf("RX_COMPLETE|FIFO_HIGH_OR_LOW, trfOffset=%i, trfRxLength=%i\r\n", trfOffset, trfRxLength);
          for (uint8_t i = trfOffset; i < trfOffset+trfRxLength; i++) {
            Log.printf(" %X", trfBuffer[i]);
          }
          Log.print("\r\n");
        */
      
      //Ghost byte workaround //TODO https://www.ti.com/lit/an/sloa140b/sloa140b.pdf 1.6
      if (trfBuffer[trfOffset] == trfBuffer[trfOffset+1] && trfBuffer[trfOffset+1] == trfBuffer[trfOffset+2]) { //Remove ghost bytes
        memmove(&trfBuffer[trfOffset], &trfBuffer[trfOffset+2], trfRxLength-2);
        trfRxLength -= 2;
      } else {
        Log.print("No ghost bytes @RX_COMPLETE|FIFO_HIGH_OR_LOW\r\n");
      }
      
      trfOffset += trfRxLength;
    } else {
        trfStatus = TRF_STATUS::PROTOCOL_ERROR;
        Log.error("Read buffer too small, size=%i, count=%", FIFO_SIZE, (trfOffset+trfRxLength));
        return;
    }
    trfStatus = TRF_STATUS::RX_WAIT_EXTENSION;
    trfRxLength = trfOffset;
  } else if (irqStatus == IRQ_STATUS::RX_COMPLETE) {
    trfRxLength = readRegister(REGISTER::FIFO_STATUS);
    trfRxLength = (0x0F & trfRxLength) + 1;
    readRegisterCont(REGISTER::FIFO, &trfBuffer[trfOffset], trfRxLength);
    /*
      Log.printf("RX_COMPLETE, trfOffset=%i, trfRxLength=%i\r\n", trfOffset, trfRxLength);
      for (uint8_t i = trfOffset; i < trfOffset+trfRxLength; i++) {
        Log.printf(" %X", trfBuffer[i]);
      }
      Log.print("\r\n");
    */
    
    //Ghost byte workaround
    if (trfBuffer[trfOffset] == trfBuffer[trfOffset+1] && trfBuffer[trfOffset+1] == trfBuffer[trfOffset+2]) { //Remove ghost bytes
      memmove(&trfBuffer[trfOffset], &trfBuffer[trfOffset+2], trfRxLength-2);
      trfRxLength -= 2;
    } else {
        Log.print("No ghost bytes @RX_COMPLETE\r\n");
    }
    
    trfOffset += trfRxLength;
    //sendCommand(DIRECT_COMMANDS::RESET_FIFO); //Really needed?
    if (trfStatus == TRF_STATUS::RX_WAIT_EXTENSION)
      trfRxLength = trfOffset;
    trfStatus = TRF_STATUS::RX_COMPLETE;
  } else if (irqStatus == (IRQ_STATUS)((uint8_t)IRQ_STATUS::RX_COMPLETE | (uint8_t)IRQ_STATUS::NO_RESPONSE)) { 
    trfStatus = TRF_STATUS::RX_WAIT_EXTENSION;
  } else if((IRQ_STATUS)((uint8_t)irqStatus & (uint8_t)IRQ_STATUS::CRC_ERROR) == IRQ_STATUS::CRC_ERROR) {
    resetRFID();
    initRFID();
    trfStatus = TRF_STATUS::PROTOCOL_ERROR;
    Log.error("CRC_ERROR not handled IRQ_STATUS=%X", irqStatus); //TODO
  } else if((IRQ_STATUS)((uint8_t)irqStatus & (uint8_t)IRQ_STATUS::FRAMING_ERROR) == IRQ_STATUS::FRAMING_ERROR) {
    if ((IRQ_STATUS)((uint8_t)irqStatus & (uint8_t)IRQ_STATUS::FIFO_HIGH_OR_LOW) == IRQ_STATUS::FIFO_HIGH_OR_LOW) {
      trfStatus = TRF_STATUS::RX_WAIT;
    } else {
      Log.error("FRAMING_ERROR not handled IRQ_STATUS=%X", irqStatus); //TODO
      resetRFID();
      initRFID();
      trfStatus = TRF_STATUS::PROTOCOL_ERROR;
    }
  } else if (irqStatus == IRQ_STATUS::IDLING) {
    trfStatus = TRF_STATUS::NO_RESPONSE_RECEIVED;
  } else if (irqStatus == IRQ_STATUS::NO_RESPONSE) {
    trfStatus = TRF_STATUS::NO_RESPONSE_RECEIVED_15693;
    trfOffset = 0;
  } else {
    trfStatus = TRF_STATUS::PROTOCOL_ERROR;
    Log.error("Unknown IRQ_STATUS=%X", irqStatus);
    //sendCommand(DIRECT_COMMANDS::RESET_FIFO);
    clearIrqRegister();
    //TODO
  }
}

void BoxRFID::clearInterrupt() {
  interrupt = false;
}
bool BoxRFID::readInterrupt() {
  return interrupt;
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

  //uint8_t res1;
  uint8_t res2;
  spiEnable();
  SPI.transfer(data);
  SPI.setDataMode(SPI_SUB_MODE_1);
  res2 = SPI.transfer(0x00); //0xFF or 0x00? (Ghost bytes)
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
    *buffer = SPI.transfer(0x00); //0xFF or 0x00? (Ghost bytes)
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
  
  //uint8_t res1, res2;
  spiEnable();
  SPI.transfer(data);
  SPI.transfer(value);
  spiDisable();

  //Log.info("Write register %i, data=%i, value=%i, res1=%i, res2=%i", regi, data, value, res1, res2);
}
void BoxRFID::sendCommand(DIRECT_COMMANDS command) {
  return sendCommand((uint8_t)command);
}
void BoxRFID::sendCommand(uint8_t command) {
  uint8_t data = command & 0b00011111;
  data |= (uint8_t)REG_CMD_WORD_BITS::COMMAND_B7 | (uint8_t)REG_CMD_WORD_BITS::WRITE_B6;

  //uint8_t res1, res2;
  spiEnable();
  SPI.transfer(data);
  SPI.transfer(0x00); //0xFF or 0x00? (Ghost bytes) //Dummy transfer, see TRF796xA SPI Design Tips (sloa140)
  spiDisable();

  //Log.info("Write command %i, data=%i, res1=%i, res2=%i", command, data, res1, res2);
}

void BoxRFID::sendRaw(uint8_t* buffer, uint8_t length) {
  const uint8_t maxFifoSize = 12;
  trfStatus = TRF_STATUS::TRF_IDLE;
  if (maxFifoSize+5 > length) {
    sendRawSPI(buffer, length, false);
  } else { //To be tested!
    uint8_t ui8TxBytesRemaining;
    uint8_t ui8TxIndex;
    uint8_t ui8FifoTxLength;
    uint8_t ui8TxBytesAvailable;
    bool bContinuedSend = false;
		ui8TxBytesRemaining = length;
		ui8TxIndex = 0;
		ui8TxBytesAvailable = maxFifoSize+5; // First send includes 5 bytes for command overhead
														// (Reset FIFO, Transmit with or without CRC, Continuous Write, Length High and Length Low)
		bContinuedSend = false;							// First send is not continued

		while(ui8TxBytesRemaining > 0) {
			if (ui8TxBytesRemaining > maxFifoSize) {
				// Avoid 60A single byte FIFO TX case from sloa140 Section 1.5
				if ((ui8TxBytesRemaining - ui8TxBytesAvailable) == 1) {
					sendRawSPI(&buffer[ui8TxIndex], ui8TxBytesAvailable-1, bContinuedSend);
					ui8TxBytesRemaining = ui8TxBytesRemaining - ui8TxBytesAvailable - 1;
				} else {
					sendRawSPI(&buffer[ui8TxIndex], ui8TxBytesAvailable, bContinuedSend);
					ui8TxBytesRemaining = ui8TxBytesRemaining - ui8TxBytesAvailable;
				}
				ui8TxIndex = ui8TxIndex + ui8TxBytesAvailable;
				bContinuedSend = true;
			} else {
				// Last send
				sendRawSPI(&buffer[ui8TxIndex], ui8TxBytesRemaining, bContinuedSend);
				bContinuedSend = false;
				ui8TxBytesRemaining = 0;
			}

      clearInterrupt();
      BoxTimer timer;
      timer.setTimer(5);
      while (!readInterrupt() && timer.isRunning()) {
        Box.delayTask(1);
        timer.tick();
      }
      if (!timer.isRunning()) {
        timeoutIRQ();
        Log.error("sendRaw Timeout");
      }

			if (trfStatus == TRF_STATUS::TX_WAIT) {
				ui8FifoTxLength = readRegister(REGISTER::FIFO_STATUS);
				ui8FifoTxLength = 0x0F & ui8FifoTxLength;
				ui8TxBytesAvailable = maxFifoSize-ui8FifoTxLength;
			} else if (trfStatus == TRF_STATUS::TX_COMPLETE) {
				if (ui8TxBytesRemaining == 0) {
					// Packet is sent
					break;
				} else {
					ui8FifoTxLength = readRegister(REGISTER::FIFO_STATUS);
					ui8FifoTxLength = 0x0F & ui8FifoTxLength;
					ui8TxBytesAvailable = maxFifoSize-ui8FifoTxLength;

					bContinuedSend = true;
				}
			} else {
				// Error occurred, break
				trfStatus = TRF_STATUS::TX_ERROR;
				break;
			}
		}
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
BoxRFID::ISO15693_RESULT BoxRFID::ISO15693_readSingleBlock(uint8_t blockId, uint8_t* blockData) {
  uint8_t offset = 0;

	trfBuffer[offset++] = 0x02;		// ISO15693 flags - ISO15693_REQ_DATARATE_HIGH
	trfBuffer[offset++] = 0x20;		// Read Single BLock
  /*
  bool withUid = true;
  if (withUid) {
    trfBuffer[0] = trfBuffer[0] || 0x20 || 0x10; // ISO15693_REQ_DATARATE_HIGH || ISO15693_REQ_ADDRESS || ISO15693_REQ_OPTION
    for (uint8_t i= 0; i<8; i++) {
      trfBuffer[offset++] = tagUid[i];
    }
  }*/
	trfBuffer[offset++] = blockId;		// BlockId

  trfStatus = sendDataTag(&trfBuffer[0], offset); 
	if (trfStatus == TRF_STATUS::RX_COMPLETE) { // If data has been received
		if (trfBuffer[0] == 0x00)	{	// Confirm "no error" in response flags byte
      if (trfRxLength == 5) {
        // data Starts at the 2rd received byte, length = 4
        for (uint8_t i=0; i<4; i++) { 
          blockData[i] = trfBuffer[i+1];
        }
        return ISO15693_RESULT::READ_SINGLE_BLOCK_VALID_RESPONSE;
      } else {
        Log.error("Invalid length, should be %i but is %i", 5, trfRxLength);
        for (uint8_t i=0; i<trfRxLength; i++) {
          Log.printf(" %x", trfBuffer[i]);
        }
        Log.println();
      }
		} else {
      Log.error("sendDataTag() Error=%X", trfStatus);
    }
	} else {
    Log.error("Unexpected TRF_STATUS=%X for read block %i", trfStatus, blockId);
	}
  return ISO15693_RESULT::READ_SINGLE_BLOCK_INVALID_RESPONSE; //TODO 
}

BoxRFID::ISO15693_RESULT BoxRFID::ISO15693_sendSingleSlotInventory(uint8_t* uid) {
  //uint8_t g_ui8TagDetectedCount;
  uint8_t ui8LoopCount = 0;
  uint8_t offset = 0;
  
	trfBuffer[offset++] = 0x26;		// ISO15693 flags
	trfBuffer[offset++] = 0x01;		// Inventory command code
	trfBuffer[offset++] = 0x00;		// Mask Length = 0 (Also not sending AFI)

  trfStatus = sendDataTag(&trfBuffer[0], offset); 
	if (trfStatus == TRF_STATUS::RX_COMPLETE) { // If data has been received
		if (trfBuffer[0] == 0x00)	{	// Confirm "no error" in response flags byte
      if (trfRxLength == 10) {
        // UID Starts at the 3rd received bit (1st is flags and 2nd is DSFID)
        for (ui8LoopCount = 2; ui8LoopCount < 10; ui8LoopCount++) {
          uid[ui8LoopCount-2] = trfBuffer[ui8LoopCount];	// Store UID into a Buffer
        }
        /*
        Log.info("RFID UID: ");
        Log.print(" ");
        for (ui8LoopCount = 0; ui8LoopCount < 8; ui8LoopCount++) {
          Log.printf("%x ", uid[7-ui8LoopCount]);		// Send UID to host
        }
        Log.println();*/
        //g_ui8TagDetectedCount = 1;
        return ISO15693_RESULT::INVENTORY_VALID_RESPONSE;
      } else {
        Log.error("Invalid length, should be %i but is %i", 10, trfRxLength);
        for (uint8_t i=0; i<trfRxLength; i++) {
          Log.printf(" %x", trfBuffer[i]);
        }
        Log.println();
      }
		} else {
      Log.error("sendDataTag() Error=%X", trfStatus);
    }
	} else {
    Log.error("Unexpected TRF_STATUS for inventory %X", trfStatus);
	}
  return ISO15693_RESULT::INVENTORY_INVALID_RESPONSE; //TODO 
}
BoxRFID::ISO15693_RESULT BoxRFID::ISO15693_getRandomSlixL(uint8_t* random) {
  uint8_t offset = 0;
  
	trfBuffer[offset++] = 0x02;		// ISO15693 flags - ISO15693_REQ_DATARATE_HIGH
	trfBuffer[offset++] = 0xB2;		// ISO15693_CMD_NXP_GET_RANDOM_NUMBER
	trfBuffer[offset++] = 0x04;		// ISO15693_MANUFACTURER_NXP

  trfStatus = sendDataTag(&trfBuffer[0], offset); 
  if (trfStatus == TRF_STATUS::RX_COMPLETE) {
		if (trfBuffer[0] == 0x00)	{	// Confirm "no error" in response flags byte
      if (trfRxLength == 3) {
        if (random) {
          random[0] = trfBuffer[1];
          random[1] = trfBuffer[2];
        }
        //uint16_t randomNum = ((trfBuffer[1]<<8)|trfBuffer[2]);
        //Log.info("Random number=%X", randomNum);
        return ISO15693_RESULT::GET_RANDOM_VALID;
      } else {
        Log.error("Invalid length, should be %i but is %i", 3, trfRxLength);
        for (uint8_t i=0; i<trfRxLength; i++) {
          Log.printf(" %x", trfBuffer[i]);
        }
        Log.println();
      }
		} else {
      Log.error("sendDataTag() Error=%X", trfStatus);
    }
  } else {
    //Log.error("Unexpected TRF_STATUS for random %X", trfStatus);
	}
  return ISO15693_RESULT::GET_RANDOM_INVALID; //TODO
}
BoxRFID::ISO15693_RESULT BoxRFID::ISO15693_setPassSlixL(uint8_t pass_id, uint32_t password) {
  uint8_t offset = 0;
  uint8_t random[2];
  
  ISO15693_RESULT result = ISO15693_getRandomSlixL(random);
  if (result != ISO15693_RESULT::GET_RANDOM_VALID)
    return result;
  
  reinitRFID();
  
  uint8_t buffer[4];
	buffer[0] = (password>>0) & 0xFF;
	buffer[1] = (password>>8) & 0xFF;
	buffer[2] = (password>>16) & 0xFF;
	buffer[3] = (password>>24) & 0xFF;

	if (random[0] || random[1]) {
		buffer[0] ^= random[0];
		buffer[1] ^= random[1];
		buffer[2] ^= random[0];
		buffer[3] ^= random[1];
	}

	trfBuffer[offset++] = 0x02;		// ISO15693 flags - ISO15693_REQ_DATARATE_HIGH
	trfBuffer[offset++] = 0xB3;		// ISO15693_CMD_NXP_SET_PASSWORD
	trfBuffer[offset++] = 0x04;		// ISO15693_MANUFACTURER_NXP
  trfBuffer[offset++] = pass_id; // Space for Password identifier

  memcpy(&trfBuffer[offset], buffer, 4);
  offset += 4; // XOR PWD
  //Log.info("Password...");
  trfStatus = sendDataTag(&trfBuffer[0], offset); 

  if (trfStatus == TRF_STATUS::RX_COMPLETE) {
		if (trfBuffer[0] == 0x00)	{	// Confirm "no error" in response flags byte
      if (trfRxLength == 1) {
        //Log.info(" ...correct");
        return ISO15693_RESULT::SET_PASSWORD_CORRECT; //TODO
      } else {
        Log.error("Invalid length, should be %i but is %i", 1, trfRxLength);
        for (uint8_t i=0; i<trfRxLength; i++) {
          Log.printf(" %x", trfBuffer[i]);
        }
        Log.println();
      }
		} else {
      Log.error("sendDataTag() Error=%X", trfStatus);
    }
  } else {
    //Log.error("Unexpected TRF_STATUS for setpwd %X", trfStatus);
	}
  //Log.error(" ...incorrect");
  return ISO15693_RESULT::SET_PASSWORD_INCORRECT; //TODO
}

void BoxRFID::reinitRFID() {
  clearInterrupt();
  trfOffset = 0;
  trfRxLength = 0;
  trfStatus = TRF_STATUS::TRF_IDLE;
  turnFieldOn();
}

uint8_t BoxRFID::readIrqRegister() {
  uint8_t buffer[2];
  buffer[0] = (uint8_t)REGISTER::IRQ_STATUS;
  buffer[1] = 0x00;
  readRegisterCont(buffer, 2);
  //Log.info("IRQ_STATUS=%X", buffer[0]);
  return buffer[0];
}
void BoxRFID::clearIrqRegister() {
  uint8_t buffer[2];
  buffer[0] = (uint8_t)REGISTER::IRQ_STATUS;
  buffer[1] = (uint8_t)REGISTER::IRQ_MASK;
  readRegisterCont(buffer, 2);
  //Log.info("IRQ_STATUS=%X", buffer[0]);
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
      Box.delayTask(1);
      timer.tick();
    }
    if (!timer.isRunning()) {
      timeoutIRQ();
      Log.error("waitTxIRQ Timeout");
    }
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
      Box.delayTask(1);
      timer.tick();
    }
    if (!timer.isRunning()) {
      timeoutIRQ();
      //Log.error("waitRxIRQ Timeout");
    }
    while (trfStatus == TRF_STATUS::RX_WAIT_EXTENSION) {
      clearInterrupt();
      timer.setTimer(5); //from firmware example
      while (!readInterrupt() && timer.isRunning()) {
        Box.delayTask(1);
        timer.tick();
      }
      if (!timer.isRunning()) {
        timeoutIRQ();
        Log.error("waitRxIRQ Timeout 2");
      }
      if (trfStatus == TRF_STATUS::NO_RESPONSE_RECEIVED)
        trfStatus = TRF_STATUS::RX_COMPLETE;
    }
    if (trfStatus == TRF_STATUS::RX_WAIT)
      trfStatus = TRF_STATUS::NO_RESPONSE_RECEIVED;
  }  
}

void BoxRFID::timeoutIRQ() {
  IRQ_STATUS irqStatus = (IRQ_STATUS)readIrqRegister();
  //Log.printf("irqStatus=%X, trfStatus=%X\r\n", irqStatus, trfStatus);
  if (irqStatus == IRQ_STATUS::TX_COMPLETE) {
    trfStatus = TRF_STATUS::TX_COMPLETE;
  } else if (irqStatus == IRQ_STATUS::IDLING) {
    trfStatus = TRF_STATUS::NO_RESPONSE_RECEIVED;
  } else {
    trfStatus = TRF_STATUS::RX_WAIT;
  }
}

void BoxRFID::resetRFID() {
  //Log.info("resetRFID();");
  sendCommand(DIRECT_COMMANDS::SOFT_INIT);
  sendCommand(DIRECT_COMMANDS::IDLING);
  delay(1); //Box.delayTask(1) --> crashes!
  clearInterrupt();
  sendCommand(DIRECT_COMMANDS::RESET_FIFO);
  trfOffset = 0;
  trfRxLength = 0;
  trfStatus = TRF_STATUS::TRF_IDLE;
}
void BoxRFID::initRFID() {
  //Log.info("initRFID();");
  /* SETUP START */
  writeRegister(REGISTER::ISO_CONTROL, 0b10000010); //ISO / IEC 15693 high bit rate, 26.48 kbps, one subcarrier, 1 out of 4 no crcr
  //writeRegister(REGISTER::ISO_CONTROL, 0b00000010); //ISO / IEC 15693 high bit rate, 26.48 kbps, one subcarrier, 1 out of 4 crcr
  writeRegister(REGISTER::IRQ_MASK, 0b00111110);
  writeRegister(REGISTER::MODULATOR_CONTROL, 0b00100001); //Sys Clock Output = 6.78MHz, OOK = 100%
  writeRegister(REGISTER::TX_PULSE_LENGTH_CONTROL, 0x80); 
  /*writeRegister(REGISTER::MODULATOR_CONTROL, 0x01); //Sys Clock Output = 13.56MHz, OOK = 100%
  writeRegister(REGISTER::RX_NO_RESPONSE_WAIT_TIME, 0x15); //No Response Wait Time
  writeRegister(REGISTER::RX_WAIT_TIME, 0x1F); //RX Wait Time*/
  /* SETUP END */
}

BoxRFID::TRF_STATUS BoxRFID::sendDataTag(uint8_t *sendBuffer, uint8_t sendLen) {
  return sendDataTag(sendBuffer, sendLen, 15, 15);  //15, 5 vs. 15, 15 (longer timeout for set password)
}
BoxRFID::TRF_STATUS BoxRFID::sendDataTag(uint8_t *sendBuffer, uint8_t sendLen, uint8_t txTimeout, uint8_t rxTimeout) {
  uint8_t buffer[sendLen+5];
  memcpy(&buffer[5], sendBuffer, sendLen);

  uint8_t offset = 0;
  buffer[offset++] = 0x8F;		// Reset FIFO
	buffer[offset++] = 0x91;		// Send  //CRC 0x91 Non CRC CRC 0x90
	buffer[offset++] = 0x3D;		// Write Continuous

	buffer[offset++] = ((sendLen>>4)&0xFF);		// Length of packet in bytes - upper and middle nibbles of transmit byte length
	buffer[offset++] = ((sendLen<<4)&0xFF);		// Length of packet in bytes - lower and broken nibbles of transmit byte length

  /*
  Log.info("sendDataTag buffer length=%i", sendLen+5);
  for (int i=0; i<sendLen+5; i++) {
    Log.printf(" %x", buffer[i]);
  }
  Log.println();*/

  sendRaw(&buffer[0], sendLen+5);
  TRF_STATUS status = waitRxData(txTimeout, rxTimeout);
  return status;
}

void BoxRFID::getUID(uint8_t* uid) {
  //size 24 (7 + 16 + 1)
  sprintf(
    (char*)uid,
    "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
    tagUid[7], tagUid[6], tagUid[5], tagUid[4], tagUid[3], tagUid[2], tagUid[1], tagUid[0]
  );
}

void BoxRFID::logUID() {
  uint8_t uid[24];
  getUID(uid);
  Log.info("RFID UID: %s", uid);
}

uint8_t BoxRFID::readBlocks(uint8_t* data, uint8_t maxBytes) {
  BoxRFID::ISO15693_RESULT result;
  uint8_t bytesRead = 0;

  resetRFID();
  initRFID();
  turnFieldOn();

  for (uint8_t i=0; i<maxBytes/4; i++) {
    result = ISO15693_readSingleBlock(i, &data[i*4]);
    if (result != ISO15693_RESULT::READ_SINGLE_BLOCK_VALID_RESPONSE)
      break;
    bytesRead += 4;
    reinitRFID();
  }

  turnFieldOff();
  return bytesRead;
}
void BoxRFID::logTagMemory() {
  uint8_t data[32];
  uint8_t bytesRead;
  bytesRead = Box.boxRFID.readBlocks(data, 32);
  if (bytesRead == 32) {
    Log.disableNewline(true);
    Log.info("Read %i bytes of memory:");
    for (uint8_t i = 0; i < bytesRead; i++) {
      Log.printf(" %x", data[i]);
    }
    Log.disableNewline(false);
    Log.println();
  } else {
    Log.error("Expected 32b but got %ib", bytesRead);
  }
}

bool BoxRFID::dumpTagMemory(bool overwrite) {
  FileFs dumpFile;
  uint8_t data[32];
  uint8_t bytesRead;
  char path[23];//= "rDUMP/0123456789ABCDEF";
  sprintf(
    (char *)path,
    "rDUMP/%02x%02x%02x%02x%02x%02x%02x%02x",
    tagUid[7], tagUid[6], tagUid[5], tagUid[4], tagUid[3], tagUid[2], tagUid[1], tagUid[0]
  );
  if (!overwrite && dumpFile.open((char*)path, FA_OPEN_EXISTING | FA_READ)) {
    dumpFile.close();
    Log.info("Dump %s exists, skip...", path);
    return false;
  }

  bytesRead = Box.boxRFID.readBlocks(data, 32);
  if (bytesRead == 32) {
    Log.disableNewline(true);
    Log.info("Read %i bytes of memory:");
    for (uint8_t i = 0; i < bytesRead; i++) {
      Log.printf(" %x", data[i]);
    }
    Log.disableNewline(false);
    Log.println();
    uint8_t mode = FA_CREATE_NEW | FA_WRITE;
    if (overwrite)
      mode = FA_CREATE_ALWAYS | FA_WRITE;

    if (dumpFile.open((char *)path, mode)) {
      dumpFile.write(data, bytesRead);
      dumpFile.close();
      Log.info("Wrote dump to %s", path);
      return true;
    } else {
      Log.error("Couldn't open %s for writing", path);
    }
  } else {
    Log.error("Expected 32b but got %ib", bytesRead);
  }
  return false;
}

void BoxRFID::turnFieldOn() {
  writeRegister(REGISTER::CHIP_STATUS_CONTROL, 0b00100001); //turnRfOn();
  // The VCD should wait at least 1 ms after it activated the
  // powering field before sending the first request, to
  // ensure that the VICCs are ready to receive it. (ISO15693-3)
  Box.delayTask(10); //not 1 ms?! 20ms works
}
void BoxRFID::turnFieldOff() {
  writeRegister(REGISTER::CHIP_STATUS_CONTROL, 0b00000001); //turnRfOff();
}