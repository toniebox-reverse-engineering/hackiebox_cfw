#ifndef BoxLEDs_h
#define BoxLEDs_h

#include "BaseHeader.h"
#include "BoxTimer.h"
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
        enum class ANIMATION_TYPE {
            SOLID,
            PULSE,
            BLINK,
            RAINBOW,
            PARTY,
        };
        enum class ANIMATION_DIRECTION {
            UP,
            DOWN,
        };
        enum class ANIMATION_COLOR {
            BLACK,
            MAROON,
            GREEN,
            OLIVE,
            NAVY,
            PURPLE,
            TEAL,
            GRAY,
            SILVER,
            RED,
            LIME,
            YELLOW,
            BLUE,
            FUCHSIA,
            AQUA,
            WHITE,
            ORANGE
        };
        struct ANIMATION {
            ANIMATION_TYPE type;
            uint8_t state;
            uint8_t step;
            ANIMATION_DIRECTION direction;
            CRGB color;
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

        void setIntervalForAnimationType(ANIMATION_TYPE idleType);
        void setIdleAnimation(ANIMATION_TYPE animationType, ANIMATION_COLOR animationColor);
        void setActiveAnimation(ANIMATION_TYPE animationType, ANIMATION_COLOR animationColor, unsigned long duration);

        void disableRedLED(bool disabled); //For SWD

    private:
        const uint8_t PIN_RED = 19;
        const uint8_t PIN_GREEN = 21;
        const uint8_t PIN_BLUE = 17;

        const uint8_t LED_PWM_MIN = 0x01;
        const uint8_t LED_PWM_MAX = 0xFE;

        uint8_t _stateRed;
        uint8_t _stateGreen;
        uint8_t _stateBlue;

        //Animation
        BoxTimer _timer;
        bool _activeAnimationRunning;
        ANIMATION _idleAnimation;
        ANIMATION _activeAnimation;
        void setAnimation(ANIMATION* animation, ANIMATION_TYPE animationType, ANIMATION_COLOR animationColor);
        CRGB _transformPulse(uint8_t state, CRGB originalColor);
        CRGB _wheel(uint8_t wheelPos);
        void _handleAnimation(ANIMATION* animation);
        CRGB _transformCRGBToAnimationColor(ANIMATION_COLOR animationColor);

        bool _redLedDisabled;
};

#endif
