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
        Log.info("Battery is critical, connect the charger, hibernating!");
        Box.boxBattery.stopBatteryTest();
        Box.boxBattery.logBatteryStatus();
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Orange, 3);
        Box.boxLEDs.waitForAnimationToFinish();
        Box.boxPower.hibernate();
        break;
    case BoxBattery::BatteryEvent::BAT_LOW:
        Log.info("Battery is low, connect the charger!");
        Box.boxBattery.logBatteryStatus();
        Box.boxLEDs.setIdleAnimation(BoxLEDs::ANIMATION_TYPE::PULSE, BoxLEDs::CRGB::Orange);
        break;
    case BoxBattery::BatteryEvent::CHR_CONNECT:
        Log.info("Charger connected");
        Box.boxBattery.logBatteryStatus();
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::White, 3);
        break;
    case BoxBattery::BatteryEvent::CHR_DISCONNECT:
        Log.info("Charger disconnected");
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
        Log.info("WiFi connected, waiting for ip...");
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Cyan, 3);
        break;
    case WrapperWiFi::ConnectionState::CONNECTED:
        Log.info("IP: %s", WiFi.localIP().toString().c_str());
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Blue, 3);
        break;
    case WrapperWiFi::ConnectionState::DISCONNECTED:
        //Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Cyan, 3);
        Log.info("WiFi lost");
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
            Log.info("Box unused, battery test running, keep alive...");  
            Box.boxPower.feedSleepTimer();
            return;
        }
        Log.info("Box unused, power off.");  
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
        Box.boxDAC.play();
        break;
    case BoxAccelerometer::Orientation::EARS_DOWN:
        orientText = "ears down";
        break;
    case BoxAccelerometer::Orientation::EARS_FRONT:
        orientText = "ears front";
        Box.boxDAC.pause();
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
    switch (event) { 
    case BoxRFID::TAG_EVENT::TAG_PLACED:
        Log.info("Tag placed", event);
        Box.boxLEDs.setIdleAnimation(BoxLEDs::ANIMATION_TYPE::PARTY, BoxLEDs::CRGB::White);
        Box.boxRFID.logUID();

        if (!Box.boxDAC.hasStopped() && (memcmp(Box.boxRFID.tagUid, Box.boxTonie.currentUid, 8) == 0)) {
            Log.info("Continue playing last file");
            Box.boxDAC.play();
        } else {
            DirFs dir; 
            if(Config.get()->misc.autodump) {
                Log.info("Autodump...");
                char* rdump = "/rDUMP";
                if (!dir.openDir(rdump)) {
                    Log.info("Create dir %s...", rdump);
                    if (!FatFs.mkdir(rdump)) {
                        Log.info("...fail!");
                    }
                }
                if (Box.boxRFID.dumpTagMemory(false)) {
                    Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Yellow, 2);
                    Box.boxDAC.beepMidi(84, 100, false);
                    Box.boxDAC.beepMidi(76, 100, false);
                } else {
                }
            } else {
                Log.info("No Autodump");
            }
            
            char* rcontent = "/rCONTENT";
            if (!dir.openDir(rcontent)) {
                Log.info("Create dir %s...", rcontent);
                if (!FatFs.mkdir(rcontent)) {
                    Log.info("...fail!");
                }
            }

            Box.boxTonie.loadTonieByUid(Box.boxRFID.tagUid);
            uint8_t* path;
            uint8_t* uid = Box.boxRFID.tagUid;
            asprintf(
                (char**)&path,
                "%s%02X%02X%02X%02X%02X%02X%02X%02X",
                Box.boxTonie.RCONTENT_BASE,
                uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]
            );
            if (!dir.openDir((char*)path)) {
                Log.info("Create dir %s...", path);
                if (!FatFs.mkdir((char*)path)) {
                    Log.info("...fail!");
                }
            } else {
                bool foundFile = false;
                while (dir.nextFile()) {
                    if (!dir.isDir()) {
                        foundFile = true;
                        break;
                    }
                }
                if (!foundFile) {
                    Log.info("No file play.");
                } else {
                    uint8_t* filepath;
                    asprintf(
                        (char**)&filepath,
                        "%s/%s",
                        path,
                        dir.fileName()
                    );
                    Box.boxDAC.playFile((const char*)filepath);
                    free(filepath);
                }
            }
            free(path);
        }
        break;
    case BoxRFID::TAG_EVENT::TAG_REMOVED:
        Log.info("Tag removed", event);
        Box.boxLEDs.defaultIdleAnimation();
        Box.boxDAC.pause();
        break;
    default:
        Log.error("Unknown TAG_EVENT=%X", event);
        break;
    }
    Box.boxPower.feedSleepTimer();
}