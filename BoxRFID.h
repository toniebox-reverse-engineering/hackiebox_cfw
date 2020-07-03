#ifndef BoxRFID_h
#define BoxRFID_h

#include "BaseHeader.h"
#include <EnhancedThread.h>
#include <SPI.h>

class BoxRFID : public EnhancedThread { 
    public:
        void
            begin(),
            loop();

        uint8_t
            readRegister(uint8_t regi);
        void
            writeRegister(uint8_t regi, uint8_t value),
            sendCommand(uint8_t command);

        void receivedInterrupt();

    private:
        enum class REG_CMD_WORD_BITS : uint8_t {
            COMMAND_B7 = 0b10000000,
            REGISTER_B7 = 0b00000000,

            READ_B6 = 0b01000000,
            WRITE_B6 = 0b00000000,

            CONTINUOUS_MODE_REG_B5 = 0b00100000
        };
        enum class DIRECT_COMMANDS : uint8_t{
            IDLING = 0x00,
            SOFT_INIT = 0x03,
            RESET_FIFO = 0x0F,
            TRANSMIT_NO_CRC = 0x10,
            TRANSMIT_CRC = 0x11,
            TRANSMIT_DELAY_NO_CRC = 0x12,
            TRANSMIT_DELAY_CRC = 0x13,
            TRANSMT_NEXT_SLOT = 0x14,
            BLOCK_RECIEVER = 0x16,
            ENABLE_RECIEVER = 0x17,
            TEST_INTERNAL_RF = 0x18,
            TEST_EXTERNAL_RF = 0x19,
            RECEIVER_GAIN_ADJ = 0x1A
        };
        enum class REGISTER : uint8_t {
            CHIP_STATUS_CONTROL = 0x00,
            ISO_CONTROL = 0x01,
            ISO14443B_TX_OPTIONS = 0x02,
            ISO14443A_BITRATE_OPTIONS = 0x03,
            TX_TIMER_EPC_HIGH = 0x04,
            TX_TIMER_EPC_LOW = 0x05,
            TX_PULSE_LENGTH_CONTROL = 0x06,
            RX_NO_RESPONSE_WAIT_TIME = 0x07,
            RX_WAIT_TIME = 0x08,
            MODULATOR_CONTROL = 0x09,
            RX_SPECIAL_SETTINGS = 0x0A,
            REGULATOR_CONTROL = 0x0B,
            IRQ_STATUS = 0x0C,
            IRQ_MASK = 0x0D,
            COLLISION_POSITION = 0x0E,
            RSSI_LEVELS = 0x0F,
            SPECIAL_FUNCTION_1 = 0x10,
            TEST_SETTINGS_1 = 0x1A,
            TEST_SETTINGS_2 = 0x1B,
            FIFO_STATUS = 0x1C,
            TX_LENGTH_BYTE_1 = 0x1D,
            TX_LENGTH_BYTE_2 = 0x1E,
            FIFO = 0x1F
        };
        enum class IRQ_STATUS : uint8_t {
            IDLING = 0x00,
            NO_RESPONSE = 0x01,
            COLLISION_ERROR = 0x02,
            FRAMING_ERROR = 0x04,
            PARITY_ERROR = 0x08,
            CRC_ERROR = 0x10,
            FIFO_HIGH_OR_LOW = 0x20,
            RX_COMPLETE = 0x40,
            TX_COMPLETE = 0x80
        };

        enum class TRF_STATUS {
            TRF_IDLE,
            TX_COMPLETE,
            RX_COMPLETE,
            TX_ERROR,
            RX_WAIT,
            RX_WAIT_EXTENSION,
            TX_WAIT,
            PROTOCOL_ERROR,
            COLLISION_ERROR,
            NO_RESPONSE_RECEIVED,
            NO_RESPONSE_RECEIVED_15693
        };

        void
            setSlaveSelect(bool enabled),
            spiEnable(),
            spiDisable();

/*
        void startReadRegister(uint8_t regi);
        void startWriteRegister(uint8_t regi);
        void endReadWriteRegister();
*/

        const uint8_t IRQ_PIN = 18;
        const static uint8_t FIFO_SIZE = 100;

        uint8_t
            readRegister(REGISTER regi);
        void
            sendCommand(DIRECT_COMMANDS command),
            writeRegister(REGISTER regi, uint8_t value);
        
        void sendRaw(uint8_t* buffer, uint8_t length);
        void sendRawSPI(uint8_t* buffer, uint8_t length, bool continuedSend);

        void
            readRegisterCont(uint8_t* buffer, uint8_t length),
            readRegisterCont(uint8_t regi, uint8_t* buffer, uint8_t length),
            readRegisterCont(REGISTER regi, uint8_t* buffer, uint8_t length);

        uint8_t readIrqRegister();

        void processInterrupt(IRQ_STATUS irqStatus);
        bool readInterrupt();
        void clearInterrupt();
        bool interrupt;

        TRF_STATUS waitRxData(uint8_t txTimeout, uint8_t rxTimeout);
        void waitTxIRQ(uint8_t txTimeout);
        void waitRxIRQ(uint8_t rxTimeout);

        bool ISO15693_sendSingleSlotInventory();

        TRF_STATUS trfStatus;
        uint8_t trfBuffer[FIFO_SIZE]; //may reduce size
        uint8_t trfOffset;
        uint8_t trfRxLength;

        void resetRFID();
};

#endif