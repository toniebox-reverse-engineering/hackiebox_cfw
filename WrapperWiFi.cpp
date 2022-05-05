#include "WrapperWiFi.h"
#include "BoxEvents.h"

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

void WrapperWiFi::begin() {
  WiFiClass::WlanProfile profiles[8];
  WiFi.getSavedProfiles(profiles);
  
  Log.info("Known WiFi Profiles:");
  for (uint8_t i=0; i<7; i++) {
    if (profiles[i].wifiNameLen>0) {
      profiles[i].wifiName[profiles[i].wifiNameLen] = '\0';
      Log.info(" -%i: Name=%s", i, profiles[i].wifiName);
    }
  }
  
  setInterval(5000);
  reconnect();
}

void WrapperWiFi::loop() {
  switch (_state) {
  case ConnectionState::WAIT_CONNECT:
    //TODO: CHECK FOR TIMEOUT / GOTO AP Mode
    if (WiFi.status() == WL_CONNECTED) {
      _state = ConnectionState::WAIT_IP;
      Events.handleWiFiEvent(_state);
      setInterval(100);
    }
    break;
  case ConnectionState::WAIT_IP:
    if (!(WiFi.localIP() == INADDR_NONE)) {
      _state = ConnectionState::CONNECTED;
      Events.handleWiFiEvent(_state);
      setInterval(5000);
    }
    break;
  case ConnectionState::CONNECTED:
    if (WiFi.status() != WL_CONNECTED) {
      _state = ConnectionState::DISCONNECTED;
      Events.handleWiFiEvent(_state);
      setInterval(5000);
    }
  default:
    break;
  }
  
}

void WrapperWiFi::reconnect() { //TODO: LED Stuff
  _state = ConnectionState::NONE;
  Log.debug("WrapperWiFi(ssid=\"%s\", password=\"%s\")", _ssid, _password);

  Log.info("Connect to WiFi %s", _ssid);
  
  //stationary mode
  if (_ip[0] != 0) {
    Log.info(" using static ip");
    WiFi.config(_ip, _dns, _subnet);
  } else {
    Log.info(" using dynamic ip");
  }
  
  WiFi.begin((char*)_ssid, (char*)_password); 
  _state = ConnectionState::WAIT_CONNECT;
  Events.handleWiFiEvent(_state);
}

void WrapperWiFi::apMode() {
  WiFi.beginNetwork("Hackiebox-");
}

WrapperWiFi::ConnectionState WrapperWiFi::getStatus() {
  return _state;
}

void WrapperWiFi::mDnsAdvertiseSetup() {
  const char* service = "Hackiebox._api._tcp.local"; //max 64 bytes?
  const char* text = "Hackiebox";

  int16_t result = sl_NetAppMDNSUnRegisterService(0, 0);

  if (result != 0)
    Log.error("mDNS service unreg 1 failed=%i", result);

  //Registering for the mDNS service.
  result = sl_NetAppMDNSUnRegisterService(
    (const signed char*)service,
    (const unsigned char)strlen(service)
  );

  if (result != 0 && result != SL_NET_APP_DNS_NOT_EXISTED_SERVICE_ERROR)
    Log.error("mDNS service unreg 2 failed=%i", result);
  
  result = sl_NetAppMDNSRegisterService(
    (const signed char*)service, //Service
    (const unsigned char)strlen(service), //ServiceLen
    (const signed char*)text, //Text
    (const unsigned char)strlen(text), //TextLen
    80, //Port
    2000, //TTL
    1 //Options
  );

  if(result == 0) {
      Log.info("mDNS service %s with %s enabled", service, text);
  } else {
      Log.error("mDNS service setup failed=%i", result);
  }
}
