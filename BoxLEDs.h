#ifndef BoxLEDs_h
#define BoxLEDs_h

#include "BaseHeader.h"

class BoxLEDs {
    public:
        void
            begin(),
            loop();
        
        void testLEDs();

        void
            setRedBool(bool power),
            setGreenBool(bool power),
            setBlueBool(bool power),
            setAllBool(bool power),
            setAllBool(bool red, bool green, bool blue);
        void
            setRed(uint8_t intensity),
            setGreen(uint8_t intensity),
            setBlue(uint8_t intensity),
            setAll(uint8_t intensity),
            setAll(uint8_t red, uint8_t green, uint8_t blue);

        uint8_t
            getRed(),
            getGreen(),
            getBlue();

    private:
        const uint8_t PIN_RED = 19;
        const uint8_t PIN_GREEN = 21;
        const uint8_t PIN_BLUE = 17;

        const uint8_t LED_PWM_MIN = 0x01;
        const uint8_t LED_PWM_MAX = 0xFE;

        uint8_t _stateRed;
        uint8_t _stateGreen;
        uint8_t _stateBlue;
};

#endif