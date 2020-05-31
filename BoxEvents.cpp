#include "BoxEvents.h"

void BoxEvents::begin() {

}
void BoxEvents::loop() {

}

void BoxEvents::handleEarEvent(BoxButtonEars::EarButton earId, BoxButtonEars::PressedType pressType, BoxButtonEars::PressedTime pressLength) {
    char* nameEar;
    char* nameType;
    char* nameLength;

    switch (earId) {
    case BoxButtonEars::EarButton::SMALL:
        nameEar = "Small ear";
        break;
    case BoxButtonEars::EarButton::BIG:
        nameEar = "Big ear";
        break;
    case BoxButtonEars::EarButton::BOTH:
        nameEar = "Both ears";
        break;
    default:
        nameEar = "Unknown";
        break;
    }

    switch (pressType) {
    case BoxButtonEars::PressedType::PRESS:
        nameType = "pressed";
        break;
    case BoxButtonEars::PressedType::RELEASE:
        nameType = "released";
        break;
    default:
        nameType = "?";
        break;
    }

    switch (pressLength) {
    case BoxButtonEars::PressedTime::SHORT:
        nameLength = "short";
        break;
    case BoxButtonEars::PressedTime::LONG:
        nameLength = "long";
        break;
    case BoxButtonEars::PressedTime::VERY_LONG:
        nameLength = "very long";
        break;
    default:
        nameLength = "unknown length";
        break;
    }

    Log.info("%s %s-%s", nameEar, nameLength, nameType);
}

void BoxEvents::handleBatteryEvent(BoxBattery::BatteryEvent state) {
    switch (state) {
    case BoxBattery::BatteryEvent::BAT_CRITICAL:
        Log.info("Battery is critical, please connect the charger, hibernating!");
        Box.boxBattery.logBatteryStatus();
        Box.boxPower.hibernate();
        break;
    case BoxBattery::BatteryEvent::BAT_LOW:
        Log.info("Battery is low, please connect the charger!");
        Box.boxBattery.logBatteryStatus();
        break;
    case BoxBattery::BatteryEvent::CHR_CONNECT:
        Log.info("Charger was connected");
        Box.boxBattery.logBatteryStatus();
        break;
    case BoxBattery::BatteryEvent::CHR_DISCONNECT:
        Log.info("Charger was disconnected");
        Box.boxBattery.logBatteryStatus();
        break;
    }
}

