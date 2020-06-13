#include "BoxButtonEars.h"
#include "BoxEvents.h"

void BoxButtonEars::begin() {  
    reloadConfig();

    _earSmall.begin();
    _earBig.begin();

    setInterval(15);
}

void BoxButtonEars::loop() { 
    _earSmall.read();
    if (_earSmall.pressedFor(_earVeryLongPressMs) && _earSmallPressedTime == PressedTime::LONG) {
        _earSmallPressedTime = PressedTime::VERY_LONG;
        if (!_earBig.isPressed() && _earBigPressedTime == PressedTime::NOT)
            Events.handleEarEvent(EarButton::SMALL, PressedType::PRESS, _earSmallPressedTime);
    } else if (_earSmall.pressedFor(_earLongPressMs) && _earSmallPressedTime == PressedTime::SHORT) {
        _earSmallPressedTime = PressedTime::LONG;
        if (!_earBig.isPressed() && _earBigPressedTime == PressedTime::NOT)
            Events.handleEarEvent(EarButton::SMALL, PressedType::PRESS, _earSmallPressedTime);
    } else if (_earSmall.isPressed() && _earSmallPressedTime == PressedTime::NOT) {
        _earSmallPressedTime = PressedTime::SHORT;
        if (!_earBig.isPressed() && _earBigPressedTime == PressedTime::NOT)
            Events.handleEarEvent(EarButton::SMALL, PressedType::PRESS, _earSmallPressedTime);
    }
    
    _earBig.read();
    if (_earBig.pressedFor(_earVeryLongPressMs) && _earBigPressedTime == PressedTime::LONG) {
        _earBigPressedTime = PressedTime::VERY_LONG;
        if (!_earSmall.isPressed() && _earSmallPressedTime == PressedTime::NOT)
            Events.handleEarEvent(EarButton::BIG, PressedType::PRESS, _earBigPressedTime);
    } else if (_earBig.pressedFor(_earLongPressMs) && _earBigPressedTime == PressedTime::SHORT) {
        _earBigPressedTime = PressedTime::LONG;
        if (!_earSmall.isPressed() && _earSmallPressedTime == PressedTime::NOT)
            Events.handleEarEvent(EarButton::BIG, PressedType::PRESS, _earBigPressedTime);
    } else if (_earBig.isPressed() && _earBigPressedTime == PressedTime::NOT) {
        _earBigPressedTime = PressedTime::SHORT;
        if (!_earSmall.isPressed() && _earSmallPressedTime == PressedTime::NOT)
            Events.handleEarEvent(EarButton::BIG, PressedType::PRESS, _earBigPressedTime);
    }

    if (_earSmallPressedTime == PressedTime::VERY_LONG && _earBigPressedTime == PressedTime::VERY_LONG && _earBothPressedTime == PressedTime::LONG) {
        _earBothPressedTime = PressedTime::VERY_LONG;
        Events.handleEarEvent(EarButton::BOTH, PressedType::PRESS, _earBothPressedTime);
    } else if (_earSmallPressedTime == PressedTime::LONG && _earBigPressedTime == PressedTime::LONG && _earBothPressedTime == PressedTime::SHORT) {
        _earBothPressedTime = PressedTime::LONG;
        Events.handleEarEvent(EarButton::BOTH, PressedType::PRESS, _earBothPressedTime);
    } else if (_earSmallPressedTime == PressedTime::SHORT && _earBigPressedTime == PressedTime::SHORT && _earBothPressedTime == PressedTime::NOT) {
        _earBothPressedTime = PressedTime::SHORT;
        Events.handleEarEvent(EarButton::BOTH, PressedType::PRESS, _earBothPressedTime);
    }
    
    if (_earBothPressedTime == PressedTime::NOT) {
        if (_earSmall.wasReleased()) {
            Events.handleEarEvent(EarButton::SMALL, PressedType::RELEASE, _earSmallPressedTime);
            _earSmallPressedTime = PressedTime::NOT;
        } else if (_earBig.wasReleased()) {
            Events.handleEarEvent(EarButton::BIG, PressedType::RELEASE, _earBigPressedTime);
            _earBigPressedTime = PressedTime::NOT;
        }
    } else if (_earSmall.wasReleased() || _earBig.wasReleased()) {
        //TODO Prevent release event of other ear
        Events.handleEarEvent(EarButton::BOTH, PressedType::RELEASE, _earBothPressedTime);
        _earBothPressedTime = PressedTime::NOT;
        _earSmallPressedTime = PressedTime::NOT;
        _earBigPressedTime = PressedTime::NOT;
    }
}

void BoxButtonEars::reloadConfig() { 
    ConfigStruct* config = Config.get();
    _earLongPressMs = config->buttonEars.longPressMs;
    _earVeryLongPressMs = config->buttonEars.veryLongPressMs;
}

void BoxButtonEars::waitForRelease() {
    while (true) {
        delay(100);
        Box.watchdog_feed();
        _earSmall.read();
        _earBig.read();
        if (!_earSmall.isPressed() && !_earBig.isPressed()) 
            break;
    }
    
}