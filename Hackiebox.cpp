#include "Hackiebox.h"
#include "crashCallback.h"

BoxConfig Config;
BoxEvents Events;
Hackiebox Box;

void crash(crashSource source, uint32_t* sp) {
    Box.logStreamSse.setSsePaused(true);
    Log.info("crashSource=%i, sp=%X, sp=%X", source, sp, (uint32_t)sp-0x20004000);

    if (source == CRASH_TEST)
      return;

    Box.boxPower.feedSleepTimer();

    FileFs _file;
    bool _isOpen;
    Log.info("Dumping SRAM 0x20004000-0x20040000 to /revvox/memdump...");
    _isOpen = _file.open("/revvox/memdump", FA_CREATE_ALWAYS | FA_WRITE);
    if (_isOpen) {
        _file.write((void *)0x20004000, 0x3C000);
        _file.close();
    }
    Log.info("...done");
    Log.info("Dumping REGISTERS 0xE000E000-0xE000F000 to /revvox/memdump...");
    _isOpen = _file.open("/revvox/regdump", FA_CREATE_ALWAYS | FA_WRITE);
    if (_isOpen) {
        _file.write((void *)0xE000E000, 0x1000);
        _file.close();
    }
    Log.info("...done");
}
void Hackiebox::setup() {  
    if (!watchdog_start()) {
        watchdog_stop();
        //reset box?!
    }
    uint32_t stackCanaries = countStackCanaries();
    uint32_t heapCanaries = countHeapCanaries();
    setCanaries();
    
    inDelayTask = true;
    logStreamMulti.setSlot(&logStreamSd, 0);
    logStreamMulti.setSlot(&logStreamSse, 1);
    Log.init(LOG_LEVEL_VERBOSE, 921600, &logStreamMulti);
    Log.info("Booting Hackiebox...");
    Box.boxPower.feedSleepTimer();
    Log.info("  -sizes: stack=%X, heap=%X", stackStart()-stackEnd(), heapEnd()-heapStart());
    Log.info("  -prev. canaries: stack=%ix, heap=%ix", stackCanaries, heapCanaries);

    register_crash_callback(crash);
    crashed(CRASH_TEST, 0);

    Wire.begin();
    
    boxPower.initPins();
    boxPower.setSdPower(true);
    boxPower.setOtherPower(true);

    boxSD.begin();
    Config.begin(); //SD Card needed!
    ConfigStruct* config = Config.get();
    
    boxPower.begin();
    boxI2C.begin();
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
    //boxDAC.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::Fuchsia);
    
    boxCLI.begin();

    boxWiFi = WrapperWiFi(config->wifi.ssid, config->wifi.password);
    boxWiFi.begin();

    webServer = WrapperWebServer();
    webServer.begin();

    boxPlayer = BoxPlayer();
    boxPlayer.begin();
    
    boxAccel.setName("Accelerometer");
    boxBattery.setName("Battery");
    boxCLI.setName("CLI");
    boxDAC.setName("DAC");
    boxRFID.setName("RFID");
    boxEars.setName("Ears");
    boxLEDs.setName("LEDs");
    boxPower.setName("Power");
    boxWiFi.setName("WiFi");
    webServer.setName("Webserver");
    
    boxDAC.priority = 0;
    boxRFID.priority = 1;
    boxAccel.priority = 2;
    boxLEDs.priority = 3;
    boxEars.priority = 4;
    webServer.priority = 5;
    boxPower.priority = 10;
    boxWiFi.priority = 50;
    boxBattery.priority = 100;
    boxCLI.priority = 100;

    threadController = ThreadController();
    threadController.add(&boxAccel);
    threadController.add(&boxBattery);
    threadController.add(&boxCLI);
    threadController.add(&boxDAC);
    threadController.add(&boxRFID);
    threadController.add(&boxEars);
    threadController.add(&boxLEDs);
    threadController.add(&boxPower);
    threadController.add(&boxWiFi);
    threadController.add(&webServer);
    threadController.sortThreads();

    Log.info("Config: %s", Config.getAsJson().c_str());

    boxAccel.onRun(ThreadCallbackHandler([&]() { boxAccel.loop(); }));
    boxCLI.onRun(ThreadCallbackHandler([&]() { boxCLI.loop(); }));
    boxDAC.onRun(ThreadCallbackHandler([&]() { boxDAC.loop(); }));
    boxRFID.onRun(ThreadCallbackHandler([&]() { boxRFID.loop(); }));
    boxPower.onRun(ThreadCallbackHandler([&]() { boxPower.loop(); }));
    boxLEDs.onRun(ThreadCallbackHandler([&]() { boxLEDs.loop(); }));
    boxBattery.onRun(ThreadCallbackHandler([&]() { boxBattery.loop(); }));
    boxEars.onRun(ThreadCallbackHandler([&]() { boxEars.loop(); }));
    boxWiFi.onRun(ThreadCallbackHandler([&]() { boxWiFi.loop(); }));
    webServer.onRun(ThreadCallbackHandler([&]() { webServer.loop(); }));

    //Dont refactor, otherwise box may crash
    boxBattery._batteryTestThread = EnhancedThread(ThreadCallbackHandler([&]() { boxBattery._doBatteryTestStep(); }), 10*60*1000);
    boxBattery._batteryTestThread.enabled = false;

    logStreamSse.setSsePaused(false);
 
    boxLEDs.defaultIdleAnimation();
    Log.info("Hackiebox started!");
    Box.boxPower.feedSleepTimer();
    inDelayTask = false;

    boxDAC.begin();
    //Workaround, as something seems to interfere / remove the irq.
    //But box now crashes!
    boxDAC.i2sStartMicros = micros();
}

void Hackiebox::delayTask(uint16_t millis) {
     if (!inDelayTask) {
        inDelayTask = true;
        BoxTimer timer;
        timer.setTimer(millis);

        //Work start
        while (timer.isRunning()) {
            delayTaskWork(timer.getTimeTillEnd());
            timer.tick();
        }
        //Work end

        inDelayTask = false;
    } else {
        delay(millis);
    }
}
void Hackiebox::delayTaskWork(uint16_t millis) {
    //delay(millis);
    boxDAC.loop(millis);
    //if (millis > 100)
    //    Log.debug("Delay %i", millis);
    //boxDAC.generateZeroAudio(millis);
}

void Hackiebox::loop() {
    watchdog_feed();
    threadController.run();
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
    MAP_WatchdogStallEnable(WDT_BASE); //Allow Debugging
    MAP_WatchdogIntRegister(WDT_BASE, watchdog_handler);
    MAP_WatchdogReloadSet(WDT_BASE, 80000000*5); //5s
    MAP_WatchdogEnable(WDT_BASE);

    return MAP_WatchdogRunning(WDT_BASE);
}
void Hackiebox::watchdog_stop() {  
    MAP_WatchdogUnlock(WDT_BASE);
    MAP_WatchdogReloadSet(WDT_BASE, 0xFFFFFFFF); //set timer to high value
    MAP_WatchdogIntClear(WDT_BASE);
    MAP_WatchdogIntUnregister(WDT_BASE);
}

