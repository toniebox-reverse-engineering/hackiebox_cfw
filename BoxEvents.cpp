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
    Box.boxPower.feedSleepTimer();
}

void BoxEvents::handleBatteryEvent(BoxBattery::BatteryEvent state) {
    switch (state) {
    case BoxBattery::BatteryEvent::BAT_CRITICAL:
        Log.info("Battery is critical, please connect the charger, hibernating!");
        Box.boxBattery.stopBatteryTest();
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

void BoxEvents::handleWiFiEvent(WrapperWiFi::ConnectionState state) {
    switch (state) {
    case WrapperWiFi::ConnectionState::WAIT_CONNECT:
        break;
    case WrapperWiFi::ConnectionState::WAIT_IP:
        Log.info("WiFi connected successfully, waiting for ip...");
        break;
    case WrapperWiFi::ConnectionState::CONNECTED:
        Log.info("IP address: %s", WiFi.localIP().toString().c_str());
        break;
    case WrapperWiFi::ConnectionState::DISCONNECTED:
        Log.info("WiFi connection lost");
        break;
    
    default:
        break;
    }
}

void BoxEvents::handlePowerEvent(BoxPower::PowerEvent event) {
    switch (event)
    {
    case BoxPower::PowerEvent::PRE_HIBERNATE:
        Log.info("Go into hibernation...");
        break;
    case BoxPower::PowerEvent::PRE_RESET:
        Log.info("Reset box...");
        break;
    case BoxPower::PowerEvent::IDLE:
        if (Box.boxBattery.batteryTestActive()) {
            Log.info("Box not used, but battery test is running, keep alive...");  
            Box.boxPower.feedSleepTimer();
            return;
        }
        
        Log.info("Box not used, powering off.");  
        Box.boxPower.hibernate();
        break;
    
    default:
        break;
    }
}

