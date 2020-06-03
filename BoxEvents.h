#ifndef BoxEvents_h
#define BoxEvents_h

#include "BaseHeader.h"
#include "Hackiebox.h"
#include "WrapperWiFi.h"
#include "BoxPower.h"

class BoxEvents {
    public:
        enum EventSource {
           BATTERY,
           EAR,
           WIFI,
           POWER
        };

        void
            begin(),
            loop();
        
        void handleEarEvent(BoxButtonEars::EarButton earId, BoxButtonEars::PressedType pressType, BoxButtonEars::PressedTime pressLength);
        void handleBatteryEvent(BoxBattery::BatteryEvent state);
        void handleWiFiEvent(WrapperWiFi::ConnectionState state);
        void handlePowerEvent(BoxPower::PowerEvent event);

    private:
};

extern BoxEvents Events;

#endif