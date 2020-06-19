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

    _activeAnimationRunning = false;
    setIdleAnimation(ANIMATION_TYPE::PULSE, ANIMATION_COLOR::TEAL);
    setActiveAnimation(ANIMATION_TYPE::BLINK, ANIMATION_COLOR::YELLOW, 10000);
}

void BoxLEDs::loop() {
    if (_activeAnimationRunning == true && _timer.isRunning()) {
        _handleAnimation(&_activeAnimation);
    } else {
        _idleAnimation.state++;
        _handleAnimation(&_idleAnimation);
    }

}

void BoxLEDs::_handleAnimation(ANIMATION* animation) {
    setIntervalForAnimationType(animation->type);
    if (animation->type == ANIMATION_TYPE::RAINBOW) {
        setAll(_wheel(animation->state));
        if (animation->state < 255) {
            animation->state = animation->state++;
        } else {
            animation->state = 0;
        }
    } else if (animation->type == ANIMATION_TYPE::PARTY) {
        setAll(_wheel(random(255)));
    } else if (animation->type == ANIMATION_TYPE::PULSE) {
        if (animation->direction == ANIMATION_DIRECTION::UP) {
            if (animation->state <= LED_PWM_MAX - animation->step) {
                animation->state = animation->state+animation->step;
            } else {
                animation->direction = ANIMATION_DIRECTION::DOWN;
                animation->state = LED_PWM_MAX;
            }
        } else { 
            if (animation->state >= LED_PWM_MIN + animation->step) {
                animation->state -= animation->step;
            } else {
                animation->direction = ANIMATION_DIRECTION::UP;
                animation->state = LED_PWM_MIN;
            }
        }
        setAll(_transformPulse(animation->state, animation->color));
    } else if (animation->type == ANIMATION_TYPE::SOLID) {
        setAll(animation->color);
    } else if (animation->type == ANIMATION_TYPE::BLINK) {
        if (animation->direction == ANIMATION_DIRECTION::UP) {
            setAll(animation->color);
            animation->direction = ANIMATION_DIRECTION::DOWN;
        } else {
            setAll(0,0,0);
            animation->direction = ANIMATION_DIRECTION::UP;
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

void BoxLEDs::setIdleAnimation(ANIMATION_TYPE animationType,ANIMATION_COLOR animationColor) {
    setAnimation(&_idleAnimation, animationType, animationColor);
}

void BoxLEDs::setActiveAnimation(ANIMATION_TYPE animationType, ANIMATION_COLOR animationColor, unsigned long duration) {
    setAnimation(&_activeAnimation, animationType, animationColor);
    _timer.setTimer(duration);
    _activeAnimationRunning = true;
}

void BoxLEDs::setAnimation(ANIMATION* animation, ANIMATION_TYPE animationType, ANIMATION_COLOR animationColor) {
    animation->type = animationType;
    animation->color = _transformCRGBToAnimationColor(animationColor);
    animation->state = 0;
    animation->step = 0;
    animation->direction = ANIMATION_DIRECTION::UP;
    setIntervalForAnimationType(animation->type);

    switch (animationType) {
    case ANIMATION_TYPE::RAINBOW:
        break;
    case ANIMATION_TYPE::BLINK:
        break;
    case ANIMATION_TYPE::PARTY:
        break;
    case ANIMATION_TYPE::PULSE:
        animation->step = 5;
        animation->direction = ANIMATION_DIRECTION::UP;
        break;
    case ANIMATION_TYPE::SOLID:
        break;
    
    default:
        break;
    }
}

BoxLEDs::CRGB BoxLEDs::_transformCRGBToAnimationColor(ANIMATION_COLOR animationColor) {
    CRGB color;
    switch (animationColor) {
    case ANIMATION_COLOR::WHITE:
        color.setRGB(0xFF,0xFF,0xFF);
        break;
    case ANIMATION_COLOR::BLACK:
        color.setRGB(0,0,0);
        break;
    case ANIMATION_COLOR::RED:
        color.setRGB(0xFF,0,0);
        break;
    case ANIMATION_COLOR::MAROON:
        color.setRGB(0x80,0,0);
        break;
    case ANIMATION_COLOR::LIME:
        color.setRGB(0,0x80,0x00);
        break;
    case ANIMATION_COLOR::GREEN:
        color.setRGB(0,0xFF,0);
        break;
    case ANIMATION_COLOR::BLUE:
        color.setRGB(0,0,0xFF);
        break;
    case ANIMATION_COLOR::NAVY:
        color.setRGB(0,0,0x80);
        break;
    case ANIMATION_COLOR::GRAY:
        color.setRGB(0x80,0x80,0x80);
        break;
    case ANIMATION_COLOR::SILVER:
        color.setRGB(0xC0,0xC0,0xC0);
        break;
    case ANIMATION_COLOR::YELLOW:
        color.setRGB(0xFF,0xFF,0);
        break;
    case ANIMATION_COLOR::ORANGE:
        color.setRGB(0xFF,0xA5,0);
        break;
    case ANIMATION_COLOR::OLIVE:
        color.setRGB(0x80,0x80,0);
        break;
    case ANIMATION_COLOR::PURPLE:
        color.setRGB(0x80,0,0x80);
        break;
    case ANIMATION_COLOR::FUCHSIA:
        color.setRGB(0xFF,0,0xFF);
        break;
    case ANIMATION_COLOR::AQUA:
        color.setRGB(0,0xFF,0xFF);
        break;
    case ANIMATION_COLOR::TEAL:
        color.setRGB(0,0x80,0x80);
        break;
    
    default:
        break;
    }
    return color;
}

void BoxLEDs::setIntervalForAnimationType(ANIMATION_TYPE idleType) {
    switch (idleType) {
    case ANIMATION_TYPE::RAINBOW:
        setInterval(100);
        break;
    case ANIMATION_TYPE::PARTY:
        setInterval(250);
        break;
    case ANIMATION_TYPE::PULSE:
        setInterval(30);
        break;
    case ANIMATION_TYPE::SOLID:
        setInterval(250);
        break;
    case ANIMATION_TYPE::BLINK:
        setInterval(350);
        break;
    
    default:
        break;
    }

}

void BoxLEDs::setRed(uint8_t intensity) {
    uint8_t currentState = _stateRed;
    if (intensity < LED_PWM_MIN) {
        _stateRed = LED_PWM_MIN;
    } else if (intensity > LED_PWM_MAX) {
        _stateRed = LED_PWM_MAX;
    } else {
        _stateRed = intensity;
    }

    if ((currentState != _stateRed) && !_redLedDisabled) {
        analogWrite(PIN_RED, _stateRed);
    }
}

void BoxLEDs::setGreen(uint8_t intensity) {
    uint8_t currentState = _stateGreen;
    if (intensity < LED_PWM_MIN) {
        _stateGreen = LED_PWM_MIN;
    } else if (intensity > LED_PWM_MAX) {
        _stateGreen = LED_PWM_MAX;
    } else {
        _stateGreen = intensity;
    }

    if (currentState != _stateGreen) {
        analogWrite(PIN_GREEN, _stateGreen);
    }
}

void BoxLEDs::setBlue(uint8_t intensity) {
    uint8_t currentState = _stateBlue;
    if (intensity < LED_PWM_MIN) {
        _stateBlue = LED_PWM_MIN;
    } else if (intensity > LED_PWM_MAX) {
        _stateBlue = LED_PWM_MAX;
    } else {
        _stateBlue = intensity;
    }

    if (currentState != _stateBlue) {
        analogWrite(PIN_BLUE, _stateBlue);
    }
}

void BoxLEDs::setRedBool(bool enabled) {
    uint8_t currentState = _stateRed;
    if (enabled) {
        _stateRed = LED_PWM_MAX;
    } else {
        _stateRed = LED_PWM_MIN;
    }
    if ((currentState != _stateRed) && !_redLedDisabled) {
        analogWrite(PIN_RED, _stateRed);
    }
}

void BoxLEDs::setGreenBool(bool enabled) {
    uint8_t currentState = _stateGreen;
    if (enabled) {
        _stateGreen = LED_PWM_MAX;
    } else {
        _stateGreen = LED_PWM_MIN;
    }

    if (currentState != _stateGreen) {
        analogWrite(PIN_GREEN, _stateGreen);
    }
}

void BoxLEDs::setBlueBool(bool enabled) {
    uint8_t currentState = _stateBlue;
    if (enabled) {
        _stateBlue = LED_PWM_MAX;
    } else {
        _stateBlue = LED_PWM_MIN;
    }

    if (currentState != _stateBlue) {
        analogWrite(PIN_BLUE, _stateBlue);
    }
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

BoxLEDs::CRGB BoxLEDs::_transformPulse(uint8_t state, CRGB originalColor) {
    CRGB color;
    color.setRGB(originalColor.red*state/255, originalColor.blue*state/255, originalColor.green*state/255);
    return color;
}
