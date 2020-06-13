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
    boxLEDs.setAllBool(true);
    boxBattery.begin();
    boxEars.begin();
    boxAccel.begin();
    boxRFID.begin();
    
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
    boxRFID.onRun(ThreadCallbackHandler([&]() { boxRFID.loop(); }));
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
    // Enable the peripherals used by this example.
    MAP_PRCMPeripheralClkEnable(PRCM_WDT, PRCM_RUN_MODE_CLK);

    // Unlock to be able to configure the registers
    MAP_WatchdogUnlock(WDT_BASE);
    
    //if (fpAppWDTCB != NULL) {
        MAP_IntPrioritySet(INT_WDT, INT_PRIORITY_LVL_1);
        MAP_WatchdogIntRegister(WDT_BASE, watchdog_handler);
    //}
    

    // Set the watchdog timer reload value
    MAP_WatchdogReloadSet(WDT_BASE, 80000000*15); //15s

    // Start the timer. Once the timer is started, it cannot be disable.
    MAP_WatchdogEnable(WDT_BASE);

    return MAP_WatchdogRunning(WDT_BASE);
}
void Hackiebox::watchdog_stop() {  
    // Unlock to be able to configure the registers
    MAP_WatchdogUnlock(WDT_BASE);

    // Disable stalling of the watchdog timer during debug events
    MAP_WatchdogStallDisable(WDT_BASE);

    // Clear the interrupt
    MAP_WatchdogIntClear(WDT_BASE);

    // Unregister the interrupt
    MAP_WatchdogIntUnregister(WDT_BASE);
}

