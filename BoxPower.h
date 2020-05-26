#ifndef BoxPower_h
#define BoxPower_h

#include "BaseHeader.h"

class BoxPower {
    public:
        void
            begin(),
            loop();
        
        void
            feedSleepTimer(),
            hibernate();

        void
            setSdPower(bool power),
            setOtherPower(bool power);

        bool
            getSdPower(),
            getOtherPower();

    private:
        uint8_t _sleepMinutes;
        uint32_t _lastFeed;
};

#endif