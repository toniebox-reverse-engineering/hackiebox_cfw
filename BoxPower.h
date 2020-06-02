#ifndef BoxPower_h
#define BoxPower_h

#include "BaseHeader.h"

#include <EnhancedThread.h>

class BoxPower : public EnhancedThread {
    public:
        void
            initPins(),
            begin(),
            loop();
        
        void
            feedSleepTimer(),
            reset(),
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
        
        void _preparePowerDown();
};

#endif