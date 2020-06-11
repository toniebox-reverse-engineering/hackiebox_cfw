#ifndef BoxLEDs_h
#define BoxLEDs_h

#include "BaseHeader.h"

#include <EnhancedThread.h>

class BoxLEDs : public EnhancedThread {
    public:
        struct CRGB {
            uint8_t red, green, blue;

            void setRGB(uint8_t r, uint8_t g, uint8_t b) {
                red = r;
                green = g;
                blue = b;
            }
        };
        enum class IDLE_TYPE {
            SOLID,
            PULSE,
            RAINBOW,
            PARTY,
        };
        enum class ANIMATION_DIRECTION {
            UP,
            DOWN,
        };
        enum class ANIMATION_COLOR {
            RED,
            GREEN,
            BLUE,
        };
        
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
            setAll(uint8_t red, uint8_t green, uint8_t blue),
            setAll(CRGB crgb);

        uint8_t
            getRed(),
            getGreen(),
            getBlue();

        void setIdleType(IDLE_TYPE idleType);

    private:
        const uint8_t PIN_RED = 19;
        const uint8_t PIN_GREEN = 21;
        const uint8_t PIN_BLUE = 17;

        const uint8_t LED_PWM_MIN = 0x01;
        const uint8_t LED_PWM_MAX = 0xFE;

        uint8_t _stateRed;
        uint8_t _stateGreen;
        uint8_t _stateBlue;

        CRGB _wheel(uint8_t wheelPos);
        uint8_t _rainbowStepState;

        IDLE_TYPE _idleType;
};

#endif
