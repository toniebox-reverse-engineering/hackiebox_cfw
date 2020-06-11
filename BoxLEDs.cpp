#include "BoxLEDs.h"
#include "wiring_private.h"

void BoxLEDs::begin() {
    PWMPrepare(PIN_RED);
    //disableRedLED(true);

    PWMPrepare(PIN_GREEN);
    PWMPrepare(PIN_BLUE);

    _stateRed = LED_PWM_MIN;
    _stateGreen = LED_PWM_MIN;
    _stateBlue = LED_PWM_MIN;

    _rainbowStepState = 0;

    _active_animation = 0;
    _animationDirection = ANIMATION_DIRECTION::UP;
    _animationState = 0;
    _animationStep = 5;
    _animationColor = ANIMATION_COLOR::RED;

}

void BoxLEDs::loop() {
    if (_active_animation == 0) {
    if (_idleType == IDLE_TYPE::RAINBOW) {
        setAll(_wheel(_rainbowStepState));
        if (_rainbowStepState < 255) {
            _rainbowStepState++;
        } else {
            _rainbowStepState = 0;
        }
    } else if (_idleType == IDLE_TYPE::PARTY) {
        setAll(_wheel(random(255)));
        } else if (_idleType == IDLE_TYPE::PULSE) {
            if (_animationDirection == ANIMATION_DIRECTION::UP) {
                if (_animationState <= LED_PWM_MAX - _animationStep) {
                    _animationState += _animationStep;
                } else {
                    _animationDirection = ANIMATION_DIRECTION::DOWN;
                    _animationState = LED_PWM_MAX;
                }
            } else { 
                if (_animationState >= LED_PWM_MIN + _animationStep) {
                    _animationState -= _animationStep;
                } else {
                    _animationDirection = ANIMATION_DIRECTION::UP;
                    _animationState = LED_PWM_MIN;
    }
}
            //Log.info("LED: pulse: state: %i", _animationState);
            switch (_animationColor) {
            case ANIMATION_COLOR::RED:
                setAll(_animationState, 0, 0);
                break;
            case ANIMATION_COLOR::GREEN:
                setAll(0, _animationState, 0);
                break;
            case ANIMATION_COLOR::BLUE:
                setAll(0,0,_animationState);
                break;
            
            default:
                break;
            }
        }
    }
}

void BoxLEDs::disableRedLED(bool disabled) {
    _redLedDisabled = disabled;
    if (disabled) {
        MAP_PinModeSet(PIN_19, PIN_MODE_1); //TCK
        MAP_PinModeSet(PIN_20, PIN_MODE_1); //TMS
    } else {
        PWMPrepare(PIN_RED);
    }
}

void BoxLEDs::disableRedLED(bool disabled) {
    _redLedDisabled = disabled;
    if (disabled) {
        MAP_PinModeSet(PIN_19, PIN_MODE_1); //TCK
        MAP_PinModeSet(PIN_20, PIN_MODE_1); //TMS
    } else {
        PWMPrepare(PIN_RED);
    }
}

void BoxLEDs:: setIdleType(IDLE_TYPE idleType) {
    _idleType = idleType;

    switch (idleType) {
    case IDLE_TYPE::RAINBOW:
        setInterval(100);
        break;
    case IDLE_TYPE::PARTY:
        setInterval(250);
        break;
    case IDLE_TYPE::PULSE:
        setInterval(30);
        break;
    
    default:
        break;
    }

}

void BoxLEDs::setRed(uint8_t intensity) {
    if (intensity < LED_PWM_MIN) {
        _stateRed = LED_PWM_MIN;
    } else if (intensity > LED_PWM_MAX) {
        _stateRed = LED_PWM_MAX;
    } else {
        _stateRed = intensity;
    }
    if (!_redLedDisabled)
        analogWrite(PIN_RED, _stateRed);
}
void BoxLEDs::setGreen(uint8_t intensity) {
    if (intensity < LED_PWM_MIN) {
        _stateGreen = LED_PWM_MIN;
    } else if (intensity > LED_PWM_MAX) {
        _stateGreen = LED_PWM_MAX;
    } else {
        _stateGreen = intensity;
    }
    analogWrite(PIN_GREEN, _stateGreen);
}
void BoxLEDs::setBlue(uint8_t intensity) {
    if (intensity < LED_PWM_MIN) {
        _stateBlue = LED_PWM_MIN;
    } else if (intensity > LED_PWM_MAX) {
        _stateBlue = LED_PWM_MAX;
    } else {
        _stateBlue = intensity;
    }
    analogWrite(PIN_BLUE, _stateBlue);
}

void BoxLEDs::setRedBool(bool enabled) {
    if (enabled) {
        _stateRed = LED_PWM_MAX;
    } else {
        _stateRed = LED_PWM_MIN;
    }
    if (!_redLedDisabled)
        analogWrite(PIN_RED, _stateRed);
}
void BoxLEDs::setGreenBool(bool enabled) {
    if (enabled) {
        _stateGreen = LED_PWM_MAX;
    } else {
        _stateGreen = LED_PWM_MIN;
    }
    analogWrite(PIN_GREEN, _stateGreen);
}
void BoxLEDs::setBlueBool(bool enabled) {
    if (enabled) {
        _stateBlue = LED_PWM_MAX;
    } else {
        _stateBlue = LED_PWM_MIN;
    }
    analogWrite(PIN_BLUE, _stateBlue);
}

uint8_t BoxLEDs::getRed() {
    return _stateRed;
}
uint8_t BoxLEDs::getGreen() {
    return _stateGreen;
}
uint8_t BoxLEDs::getBlue() {
    return _stateBlue;
}

void BoxLEDs::setAllBool(bool enabled) {
    setAllBool(enabled, enabled, enabled);
}
void BoxLEDs::setAllBool(bool red, bool green, bool blue) {
    setRedBool(red);
    setGreenBool(green);
    setBlueBool(blue);
}
void BoxLEDs::setAll(uint8_t intensity) {
    setAll(intensity, intensity, intensity);
}
void BoxLEDs::setAll(uint8_t red, uint8_t green, uint8_t blue) {
    setRed(red);
    setGreen(green);
    setBlue(blue);
}
void BoxLEDs::setAll(CRGB crgb) {
    setAll(crgb.red, crgb.green, crgb.blue);
}

void BoxLEDs::testLEDs() {
    uint8_t ledR = getRed();
    uint8_t ledG = getGreen();
    uint8_t ledB = getBlue();

    Log.info("Testing LEDs...");
    delay(250);

    Log.info(" Red");
    setAll(0xFF, 0x00, 0x00);
    delay(250);
    setAll(0x7F, 0x00, 0x00);
    delay(250);
    Log.info(" Green");
    setAll(0x00, 0xFF, 0x00);
    delay(250);
    setAll(0x00, 0x7F, 0x00);
    delay(250);
    Log.info(" Blue");
    setAll(0x00, 0x00, 0xFF);
    delay(250);
    setAll(0x00, 0x00, 0x7F);
    delay(250);
    Log.info(" RGB");
    setAll(0xFF, 0xFF, 0xFF);
    delay(250);
    setAll(0x7F, 0x7F, 0x7F);
    delay(250);

    Log.info(" Off");
    setAll(0x00);

    delay(500);
    Log.info(" Reset");
    setAll(ledR, ledG, ledB);
    Log.info(" Test finished.");
}

BoxLEDs::CRGB BoxLEDs::_wheel(uint8_t wheelPos) {
  CRGB color;
  if (wheelPos < 85) {
   color.setRGB(wheelPos * 3, 255 - wheelPos * 3, 0);
  } else if (wheelPos < 170) {
   wheelPos -= 85;
   color.setRGB(255 - wheelPos * 3, 0, wheelPos * 3);
  } else {
   wheelPos -= 170; 
   color.setRGB(0, wheelPos * 3, 255 - wheelPos * 3);
  }
  return color;
}
