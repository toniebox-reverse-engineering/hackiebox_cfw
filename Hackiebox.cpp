#include "Hackiebox.h"

void Hackiebox::setup() {  
    Log.init(LOG_LEVEL_VERBOSE, 115200);
    Log.info("Booting Hackiebox...");

    _wifi = WrapperWiFi(WIFI_SSID, WIFI_PASS);
    _wifi.begin();

    _server = WrapperWebServer();
    _server.begin();
}
void Hackiebox::loop() {  
    _server.handle();
}