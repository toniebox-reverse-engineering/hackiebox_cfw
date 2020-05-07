#ifndef BoxPower_h
#define BoxPower_h

#include "BaseHeader.h"

class BoxPower {
    public:
        void
            begin(),
            loop();
        
        void
            setSdPower(bool power),
            setOtherPower(bool power);

        bool
            getSdPower(),
            getOtherPower();

    private:
};

#endif