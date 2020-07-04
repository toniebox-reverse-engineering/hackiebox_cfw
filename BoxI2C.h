#ifndef BoxI2C_h
#define BoxI2C_h

#include "BaseHeader.h"
#include <Wire.h>

class BoxI2C {
    public:
        void
            begin();

        bool send(uint8_t address, uint8_t target_register, uint8_t data);
        bool send_raw(uint8_t data);
        uint8_t readByte(uint8_t address, uint8_t source_register);
};

#endif