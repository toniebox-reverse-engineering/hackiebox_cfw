#ifndef BoxTimer_h
#define BoxTimer_h

#include "BaseHeader.h"

class BoxTimer {
    public:
        void setTimer(unsigned long milliseconds);
        void stopTimer();
        bool isRunning();
    private:
        bool _isRunning;
        unsigned long _endMillis;
};

#endif
