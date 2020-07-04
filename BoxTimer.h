#ifndef BoxTimer_h
#define BoxTimer_h

#include "BaseHeader.h"

class BoxTimer {
    public:
        void setTimer(unsigned long milliseconds);
        void stopTimer();
        void tick();
        bool isRunning();
        bool wasRunning();
    private:
        bool _isRunning;
        bool _hasChanged;
        unsigned long _endMillis;
};

#endif
