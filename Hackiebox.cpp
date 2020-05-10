#include "Hackiebox.h"


BoxConfig Config;
void Hackiebox::setup() {  
    Log.init(LOG_LEVEL_VERBOSE, 115200);
    Log.info("Booting Hackiebox...");

    _boxPower.begin();
    _boxPower.setSdPower(true);
    _boxPower.setOtherPower(true);

    Config.begin(); //SD Card needed!

    _boxLEDs.begin();
    _boxLEDs.testLEDs();

    _boxBattery.begin();
    _boxBattery.loop();

    _boxEars.begin();


    
    _boxLEDs.setBlue(true);
    _wifi = WrapperWiFi(WIFI_SSID, WIFI_PASS);
    _wifi.begin();

    _server = WrapperWebServer();
    _server.begin();
    _boxLEDs.setBlue(false);
    _boxLEDs.setGreen(true);
    

    Log.info("Hackiebox started!");

    Log.info("Config: %s", Config.getAsJson().c_str());
}

void Hackiebox::loop() {  
    _boxBattery.loop();
    _boxEars.loop();
    _boxPower.loop();
    _server.handle();
}