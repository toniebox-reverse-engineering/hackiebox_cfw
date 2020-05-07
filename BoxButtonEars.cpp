#include "BoxButtonEars.h"

void BoxButtonEars::begin() {  
    _earSmall.begin();
    _earBig.begin();
}

void BoxButtonEars::loop() { 
    _earSmall.read();
    
    if (_earSmall.pressedFor(_earVeryLongPressMs) && _earSmallPressedTime == LONG) {
        Log.info("Small ear very-long-pressed.");
        _earSmallPressedTime = VERY_LONG;
    } else if (_earSmall.pressedFor(_earLongPressMs) && _earSmallPressedTime == SHORT) {
        Log.info("Small ear long-pressed.");
        _earSmallPressedTime = LONG;
    } else if (_earSmall.isPressed() && _earSmallPressedTime == NOT) {
        Log.info("Small ear short-pressed.");
        _earSmallPressedTime = SHORT;
    }
    
    _earBig.read();
    if (_earBig.pressedFor(_earVeryLongPressMs) && _earBigPressedTime == LONG) {
        Log.info("Big ear very-long-pressed.");
        _earBigPressedTime = VERY_LONG;
    } else if (_earBig.pressedFor(_earLongPressMs) && _earBigPressedTime == SHORT) {
        Log.info("Big ear long-pressed.");
        _earBigPressedTime = LONG;
    } else if (_earBig.isPressed() && _earBigPressedTime == NOT) {
        Log.info("Big ear short-pressed.");
        _earBigPressedTime = SHORT;
    }

    if (_earSmallPressedTime == VERY_LONG && _earBigPressedTime == VERY_LONG && _earBothPressedTime == LONG) {
        Log.info("Both ears very-long-pressed.");
        _earBothPressedTime = VERY_LONG;
    } else if (_earSmallPressedTime == LONG && _earBigPressedTime == LONG && _earBothPressedTime == SHORT) {
        Log.info("Both ears long-pressed.");
        _earBothPressedTime = LONG;
    } else if (_earSmallPressedTime == SHORT && _earBigPressedTime == SHORT && _earBothPressedTime == NOT) {
        Log.info("Both ears short-pressed.");
        _earBothPressedTime = SHORT;
    }
    
    if (_earBothPressedTime == NOT) {
        if (_earSmall.wasReleased()) {
            switch (_earSmallPressedTime) {
            case VERY_LONG:
                Log.info("Small ear very-long-released.");
                break;
            case LONG:
                Log.info("Small ear long-released.");
                break;
            case SHORT:
                Log.info("Small ear short-released.");
                break;
            }
            _earSmallPressedTime = NOT;
        } else if (_earBig.wasReleased()) {
            switch (_earBigPressedTime) {
            case VERY_LONG:
                Log.info("Big ear very-long-released.");
                break;
            case LONG:
                Log.info("Big ear long-released.");
                break;
            case SHORT:
                Log.info("Big ear short-released.");
                break;
            }
            _earBigPressedTime = NOT;
        }
    } else if (_earSmall.wasReleased() || _earBig.wasReleased()) {
        //TODO!!!
        switch (_earBothPressedTime) {
        case VERY_LONG:
            Log.info("Big ear very-long-released.");
            break;
        case LONG:
            Log.info("Big ear long-released.");
            break;
        case SHORT:
            Log.info("Big ear short-released.");
            break;
        }
        _earBothPressedTime = NOT;
        _earSmallPressedTime = NOT;
        _earBigPressedTime = NOT;
    }
}