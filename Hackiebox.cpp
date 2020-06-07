#include "Hackiebox.h"

BoxConfig Config;
BoxEvents Events;
Hackiebox Box;
void Hackiebox::setup() {  
    //watchdog set 30s?

    Log.init(LOG_LEVEL_VERBOSE, 115200);
    Log.info("Booting Hackiebox, Free MEM=%ib...", freeMemory());

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
    boxAccel.begin();
    
    boxWiFi = WrapperWiFi(config->wifi.ssid, config->wifi.password);
    boxWiFi.begin();

    webServer = WrapperWebServer();
    webServer.begin();
    
    _threadController = ThreadController();
    _threadController.add(&boxAccel);
    _threadController.add(&boxBattery);
    _threadController.add(&boxEars);
    _threadController.add(&boxLEDs);
    _threadController.add(&boxPower);
    _threadController.add(&boxWiFi);
    _threadController.add(&webServer);

    Log.info("Config: %s", Config.getAsJson().c_str());

    boxAccel.onRun(ThreadCallbackHandler([&]() { boxAccel.loop(); }));
    boxPower.onRun(ThreadCallbackHandler([&]() { boxPower.loop(); }));
    boxLEDs.onRun(ThreadCallbackHandler([&]() { boxLEDs.loop(); }));
    boxBattery.onRun(ThreadCallbackHandler([&]() { boxBattery.loop(); }));
    boxEars.onRun(ThreadCallbackHandler([&]() { boxEars.loop(); }));
    boxWiFi.onRun(ThreadCallbackHandler([&]() { boxWiFi.loop(); }));
    webServer.onRun(ThreadCallbackHandler([&]() { webServer.loop(); }));

    boxBattery._batteryTestThread = EnhancedThread(ThreadCallbackHandler([&]() { boxBattery._doBatteryTestStep(); }), 10*60*1000);
    boxBattery._batteryTestThread.enabled = false;
 
    Log.info("Hackiebox started! Free MEM=%ib...", freeMemory());
}

void Hackiebox::loop() {  
    _threadController.run();

    //watchdog.feed
    webServer.handle();
}