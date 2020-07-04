#include "BoxTimer.h"

void BoxTimer::setTimer(unsigned long milliseconds) { 
    _endMillis = millis() + milliseconds;
    _isRunning = true;
    _hasChanged = true;
}

void BoxTimer::tick() {
    if (!_isRunning) {
        _hasChanged = false;
    } else {
        unsigned long currentMillis = millis();
        if (_endMillis <= currentMillis) {
            _isRunning = false;
            _hasChanged = true;
        }
    }
}

bool BoxTimer::isRunning() { 
    return _isRunning;
}

bool BoxTimer::wasRunning() { 
    return !_isRunning && _hasChanged;
}

void BoxTimer::stopTimer() {
   _isRunning = false; 
}
