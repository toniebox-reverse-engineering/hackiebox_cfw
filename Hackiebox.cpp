#include "Hackiebox.h"

BoxConfig Config;
Hackiebox Box;
void Hackiebox::setup() {  
    Log.init(LOG_LEVEL_VERBOSE, 115200);
    Log.info("Booting Hackiebox...");

    boxPower.begin();
    boxPower.setSdPower(true);
    boxPower.setOtherPower(true);

    boxSD.begin();
    Config.begin(); //SD Card needed!
    ConfigStruct* config = Config.get();
    
    boxLEDs.begin();
    boxLEDs.testLEDs();

    boxBattery.begin();
    boxBattery.loop();

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
    boxBattery.loop();
    boxEars.loop();
    boxPower.loop();
    _server.handle();
}