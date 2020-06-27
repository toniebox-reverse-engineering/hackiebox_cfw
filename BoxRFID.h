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
        bool
            writeRegister(uint8_t regi, uint8_t value),
            sendCommand(uint8_t command);

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
            TRANSMT_NEXT_SLOT = 0x14,
            BLOCK_RECIEVER = 0x16,
            ENABLE_RECIEVER = 0x17,
            TEST_INTERNAL_RF = 0x18,
            TEST_EXTERNAL_RF = 0x19,
            RECEIVER_GAIN_ADJ = 0x1A
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

        bool sendCommand(DIRECT_COMMANDS command);
};

#endif