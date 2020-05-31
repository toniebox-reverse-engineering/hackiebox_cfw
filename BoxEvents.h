#ifndef BoxEvents_h
#define BoxEvents_h

#include "BaseHeader.h"

#include "BoxButtonEars.h"

class BoxEvents {
    public:
        enum EventSource {
           BATTERY,
           EAR,
        };

        void
            begin(),
            loop();
        
        void handleEarEvent(BoxButtonEars::EarButton earId, BoxButtonEars::PressedType pressType, BoxButtonEars::PressedTime pressLength);

    private:
};

extern BoxEvents Events;

#endif