#include "WrapperWiFi.h"

WrapperWiFi::WrapperWiFi(const char* ssid, const char* password) {  
  _ssid = ssid;
  _password = password;
  byte empty[4] = {0};
  memcpy(_ip, empty, sizeof(_ip));
  memcpy(_subnet, empty, sizeof(_subnet));
  memcpy(_dns, empty, sizeof(_dns));
}

WrapperWiFi::WrapperWiFi(const char* ssid, const char* password, const byte ip[4], const byte subnet[4], const byte dns[4]) {  
  _ssid = ssid;
  _password = password;
  if (ip[0] != 0) {
    memcpy(_ip, ip, sizeof(_ip));
    memcpy(_subnet, subnet, sizeof(_subnet));
    memcpy(_dns, dns, sizeof(_dns));
  } else {
    byte empty[4] = {0};
    memcpy(_ip, empty, sizeof(_ip));
    memcpy(_subnet, empty, sizeof(_subnet));
    memcpy(_dns, empty, sizeof(_dns));
  }
}

void WrapperWiFi::begin(void) {
  
  Log.debug("WrapperWiFi(ssid=\"%s\", password=\"%s\")", _ssid, _password);

  Log.info("Connecting to WiFi %s", _ssid);
  
  //stationary mode
  if (_ip[0] != 0) {
    Log.info("Using static ip");
    WiFi.config(_ip, _dns, _subnet);
  } else {
    Log.info("Using dynamic ip");
  }
  
 WiFi.begin((char*)_ssid, (char*)_password); 
  while (WiFi.status() != WL_CONNECTED) {
    //Log.error("WiFi Connection Failed!");
    //TODO AP Mode
  }
  Log.info("Connected successfully");
  while (WiFi.localIP() == INADDR_NONE) {
    
  }
  Log.info("IP address: %s", WiFi.localIP().toString().c_str());
  //Log.info("IP address received");
  
}
