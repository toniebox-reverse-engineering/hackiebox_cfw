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
            setRed(bool power),
            setGreen(bool power),
            setBlue(bool power),
            setAll(bool power);

        bool
            getRed(),
            getGreen(),
            getBlue();

    private:
        uint8_t _stateRed;
        uint8_t _stateGreen;
        uint8_t _stateBlue;
};

#endif