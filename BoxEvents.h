#ifndef BoxEvents_h
#define BoxEvents_h

#include "BaseHeader.h"
#include "Hackiebox.h"

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
        void handleBatteryEvent(BoxBattery::BatteryEvent state);

    private:
};

extern BoxEvents Events;

#endif