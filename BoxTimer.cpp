#include "BoxTimer.h"

void BoxTimer::setTimer(unsigned long milliseconds) { 
    _endMillis = millis() + milliseconds;
    _isRunning = true;
}

bool BoxTimer::isRunning() { 
    if (!_isRunning)
        return false;

    unsigned long currentMillis = millis();
    if (_endMillis <= currentMillis)
        _isRunning = false;

    return _isRunning;
}

void BoxTimer::stopTimer() {
   _isRunning = false; 
}
