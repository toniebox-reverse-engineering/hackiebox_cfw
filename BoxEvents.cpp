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

    if (pressType == BoxButtonEars::PressedType::PRESS) {
        if (pressLength == BoxButtonEars::PressedTime::SHORT) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                Box.boxDAC.increaseVolume();
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                Box.boxDAC.decreaseVolume();
            } else if (earId == BoxButtonEars::EarButton::BOTH) {
                
            }
        } else if (pressLength == BoxButtonEars::PressedTime::LONG) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                if (Box.boxWiFi.getStatus() != WrapperWiFi::ConnectionState::CONNECTED)
                    Box.boxWiFi.reconnect();
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                
            } else if (earId == BoxButtonEars::EarButton::BOTH) {
                
            }
        } else if (pressLength == BoxButtonEars::PressedTime::VERY_LONG) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                
            } else if (earId == BoxButtonEars::EarButton::BOTH) {
                if (Box.boxAccel.getOrientation() == BoxAccelerometer::Orientation::EARS_UP) {
                    Box.boxLEDs.setIdleAnimation(BoxLEDs::ANIMATION_TYPE::PULSE, BoxLEDs::CRGB::Blue);
                    Box.boxWiFi.apMode();
                } else if (Box.boxAccel.getOrientation() == BoxAccelerometer::Orientation::EARS_DOWN) {
                    Box.boxPower.reset();
                } else if (Box.boxAccel.getOrientation() == BoxAccelerometer::Orientation::EARS_FRONT) {
                    //Prepare Hibernation
                    Box.boxLEDs.setAllBool(false);
                    Box.boxEars.waitForRelease();
                    delay(500);
                    Box.boxPower.hibernate();
                }
            }
        }
    } else if (pressType == BoxButtonEars::PressedType::RELEASE) {
        if (pressLength == BoxButtonEars::PressedTime::SHORT) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                
            } else if (earId == BoxButtonEars::EarButton::BOTH) {
                
            }
        } else if (pressLength == BoxButtonEars::PressedTime::LONG) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                if (Box.boxWiFi.getStatus() != WrapperWiFi::ConnectionState::CONNECTED)
                    Box.boxWiFi.reconnect();
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                
            } else if (earId == BoxButtonEars::EarButton::BOTH) {
                
            }
        } else if (pressLength == BoxButtonEars::PressedTime::VERY_LONG) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                
            } else if (earId == BoxButtonEars::EarButton::BOTH) {

            }
        }
    }
}

void BoxEvents::handleBatteryEvent(BoxBattery::BatteryEvent state) {
    switch (state) {
    case BoxBattery::BatteryEvent::BAT_CRITICAL:
        Log.info("Battery is critical, please connect the charger, hibernating!");
        Box.boxBattery.stopBatteryTest();
        Box.boxBattery.logBatteryStatus();
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Orange, 3);
        Box.boxLEDs.waitForAnimationToFinish();
        Box.boxPower.hibernate();
        break;
    case BoxBattery::BatteryEvent::BAT_LOW:
        Log.info("Battery is low, please connect the charger!");
        Box.boxBattery.logBatteryStatus();
        Box.boxLEDs.setIdleAnimation(BoxLEDs::ANIMATION_TYPE::PULSE, BoxLEDs::CRGB::Orange);
        break;
    case BoxBattery::BatteryEvent::CHR_CONNECT:
        Log.info("Charger was connected");
        Box.boxBattery.logBatteryStatus();
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::White, 3);
        break;
    case BoxBattery::BatteryEvent::CHR_DISCONNECT:
        Log.info("Charger was disconnected");
        Box.boxBattery.logBatteryStatus();
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::DarkSlateGray, 3);
        break;
    }
}

void BoxEvents::handleWiFiEvent(WrapperWiFi::ConnectionState state) {
    switch (state) {
    case WrapperWiFi::ConnectionState::WAIT_CONNECT:
        break;
    case WrapperWiFi::ConnectionState::WAIT_IP:
        Log.info("WiFi connected successfully, waiting for ip...");
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Cyan, 3);
        break;
    case WrapperWiFi::ConnectionState::CONNECTED:
        Log.info("IP address: %s", WiFi.localIP().toString().c_str());
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Blue, 3);
        break;
    case WrapperWiFi::ConnectionState::DISCONNECTED:
        //Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Cyan, 3);
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
    case BoxPower::PowerEvent::BOX_IDLE:
        if (Box.boxBattery.batteryTestActive()) {
            Log.info("Box not used, but battery test is running, keep alive...");  
            Box.boxPower.feedSleepTimer();
            return;
        }
        Log.info("Box not used, powering off.");  
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Green, 3);
        Box.boxLEDs.waitForAnimationToFinish();
        Box.boxPower.hibernate();
        break;
    
    default:
        break;
    }
}

void BoxEvents::handleAccelerometerOrientationEvent(BoxAccelerometer::Orientation orient) {
    char* orientText;
    switch (orient) {
    case BoxAccelerometer::Orientation::EARS_UP:
        orientText = "ears up";
        break;
    case BoxAccelerometer::Orientation::EARS_DOWN:
        orientText = "ears down";
        break;
    case BoxAccelerometer::Orientation::EARS_FRONT:
        orientText = "ears front";
        break;
    case BoxAccelerometer::Orientation::EARS_BACK:
        orientText = "ears back";
        break;
    case BoxAccelerometer::Orientation::EARS_LEFT:
        orientText = "ears left";
        break;
    case BoxAccelerometer::Orientation::EARS_RIGHT:
        orientText = "ears right";
        break;
    case BoxAccelerometer::Orientation::OTHER:
        orientText = "lockout";
        break;
    
    default:
        orientText = "unknown";
        Log.error("Unknown orientation=%i", orient);
        break;
    }
    Log.info("Box' orientation changed to %s", orientText);
    Box.boxPower.feedSleepTimer();
}

void BoxEvents::handleTagEvent(BoxRFID::TAG_EVENT event) {
    BoxTonies tonie;

    switch (event) { 
    case BoxRFID::TAG_EVENT::TAG_PLACED:
        Log.info("Tag placed", event);
        Box.boxLEDs.setIdleAnimation(BoxLEDs::ANIMATION_TYPE::PARTY, BoxLEDs::CRGB::White);
        Box.boxRFID.logUID();
        tonie.loadTonieByUid(Box.boxRFID.tagUid);
        break;
    case BoxRFID::TAG_EVENT::TAG_REMOVED:
        Log.info("Tag removed", event);
        Box.boxLEDs.defaultIdleAnimation();
        break;
    default:
        Log.error("Unknown TAG_EVENT=%X", event);
        break;
    }
    Box.boxPower.feedSleepTimer();
}