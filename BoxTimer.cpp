#include "BoxTimer.h"

void BoxTimer::setTimer(unsigned long milliseconds) { 
    _currentMillis = millis();
    _endMillis = _currentMillis + milliseconds;
    _isRunning = true;
    _hasChanged = true;
}

void BoxTimer::tick() {
    if (!_isRunning) {
        _hasChanged = false;
    } else {
        _currentMillis = millis();
        if (_endMillis <= _currentMillis) {
            _isRunning = false;
            _hasChanged = true;
        }
    }
}

unsigned long BoxTimer::getTimeTillEnd() {
    if (!_isRunning)
        return 0;
    return _endMillis - _currentMillis;
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
