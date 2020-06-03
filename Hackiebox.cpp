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
    boxLEDs.begin();
    boxLEDs.setAllBool(true);
    boxBattery.begin();
    boxEars.begin();
    
    _wifi = WrapperWiFi(config->wifi.ssid, config->wifi.password);
    _wifi.begin();

    _server = WrapperWebServer();
    _server.begin();
    
    _threadController = ThreadController();
    _threadController.add(&boxBattery);
    _threadController.add(&boxEars);
    _threadController.add(&boxLEDs);
    _threadController.add(&boxPower);
    _threadController.add(&_wifi);
 
    Log.info("Hackiebox started!");

    Log.info("Config: %s", Config.getAsJson().c_str());
    Log.info("SD root: %s", boxSD.jsonListDir("/").c_str());


    boxPower.onRun(ThreadCallbackHandler([&]() { boxPower.loop(); }));
    boxLEDs.onRun(ThreadCallbackHandler([&]() { boxLEDs.loop(); }));
    boxBattery.onRun(ThreadCallbackHandler([&]() { boxBattery.loop(); }));
    boxEars.onRun(ThreadCallbackHandler([&]() { boxEars.loop(); }));
    _wifi.onRun(ThreadCallbackHandler([&]() { _wifi.loop(); }));

}

void Hackiebox::loop() {  
    _threadController.run();

    //watchdog.feed
    _server.handle();
}