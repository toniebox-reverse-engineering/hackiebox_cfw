#include "Hackiebox.h"

BoxConfig Config;
BoxEvents Events;
Hackiebox Box;
void Hackiebox::setup() {  
    if (!watchdog_start()) {
        watchdog_stop();
        //reset box?!
    }

    Log.init(LOG_LEVEL_VERBOSE, 115200/*, &logStream*/);
    Log.info("Booting Hackiebox, Free MEM=%ib...", freeMemory());

    boxPower.initPins();
    boxPower.setSdPower(true);
    boxPower.setOtherPower(true);

    boxSD.begin();
    Config.begin(); //SD Card needed!
    ConfigStruct* config = Config.get();
    
    boxPower.begin();
    boxLEDs.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::White);
    boxBattery.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::Orange);
    boxEars.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::Yellow);
    boxAccel.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::Pink);
    boxRFID.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::Teal);
    boxDAC.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::Fuchsia);
    
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
    boxDAC.onRun(ThreadCallbackHandler([&]() { boxDAC.loop(); }));
    boxRFID.onRun(ThreadCallbackHandler([&]() { boxRFID.loop(); }));
    boxPower.onRun(ThreadCallbackHandler([&]() { boxPower.loop(); }));
    boxLEDs.onRun(ThreadCallbackHandler([&]() { boxLEDs.loop(); }));
    boxBattery.onRun(ThreadCallbackHandler([&]() { boxBattery.loop(); }));
    boxEars.onRun(ThreadCallbackHandler([&]() { boxEars.loop(); }));
    boxWiFi.onRun(ThreadCallbackHandler([&]() { boxWiFi.loop(); }));
    webServer.onRun(ThreadCallbackHandler([&]() { webServer.loop(); }));

    boxBattery._batteryTestThread = EnhancedThread(ThreadCallbackHandler([&]() { boxBattery._doBatteryTestStep(); }), 10*60*1000);
    boxBattery._batteryTestThread.enabled = false;
 
    boxLEDs.setIdleAnimation(BoxLEDs::ANIMATION_TYPE::RAINBOW, BoxLEDs::CRGB::White);
    Log.info("Hackiebox started! Free MEM=%ib...", freeMemory());
}

void Hackiebox::loop() {  
    watchdog_feed();
    _threadController.run();
    webServer.handle();
}

bool Hackiebox::watchdog_isFed() {
    return _watchdog_fed;
}
void Hackiebox::watchdog_feed() {
    _watchdog_fed = true;
}
void Hackiebox::watchdog_unfeed() {
    _watchdog_fed = false;
}
void watchdog_handler() {
    if (Box.watchdog_isFed()) {
        MAP_WatchdogIntClear(WDT_BASE);
        Box.watchdog_unfeed();
    }
}
bool Hackiebox::watchdog_start() {
    watchdog_feed();

    MAP_PRCMPeripheralClkEnable(PRCM_WDT, PRCM_RUN_MODE_CLK);
    MAP_WatchdogUnlock(WDT_BASE);
    MAP_IntPrioritySet(INT_WDT, INT_PRIORITY_LVL_1);
    MAP_WatchdogIntRegister(WDT_BASE, watchdog_handler);
    MAP_WatchdogReloadSet(WDT_BASE, 80000000*15); //15s
    MAP_WatchdogEnable(WDT_BASE);

    return MAP_WatchdogRunning(WDT_BASE);
}
void Hackiebox::watchdog_stop() {  
    MAP_WatchdogUnlock(WDT_BASE);
    MAP_WatchdogStallDisable(WDT_BASE);
    MAP_WatchdogIntClear(WDT_BASE);
    MAP_WatchdogIntUnregister(WDT_BASE);
}

