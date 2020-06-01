#include "Hackiebox.h"

BoxConfig Config;
BoxEvents Events;
Hackiebox Box;
void Hackiebox::setup() {  
    //watchdog set 30s?

    Log.init(LOG_LEVEL_VERBOSE, 115200);
    Log.info("Booting Hackiebox...");

    boxPower.initPins();
    boxPower.setSdPower(true);
    boxPower.setOtherPower(true);

    boxSD.begin();
    Config.begin(); //SD Card needed!
    ConfigStruct* config = Config.get();
    
    boxPower.begin();

    boxBattery.begin();

    boxLEDs.begin();
    boxLEDs.testLEDs();

    boxEars.begin();
    
    boxLEDs.setBlue(true);
    _wifi = WrapperWiFi(config->wifi.ssid, config->wifi.password);
    _wifi.begin();

    _server = WrapperWebServer();
    _server.begin();
    boxLEDs.setBlue(false);
    boxLEDs.setGreen(true);
    

    Log.info("Hackiebox started!");

    Log.info("Config: %s", Config.getAsJson().c_str());
    Log.info("SD root: %s", boxSD.jsonListDir("/").c_str());
}

void Hackiebox::loop() {  
    //watchdog.feed

    boxBattery.loop();
    boxEars.loop();
    boxPower.loop();
    _server.handle();
}