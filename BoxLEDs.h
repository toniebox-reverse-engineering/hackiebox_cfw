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
};

#endif