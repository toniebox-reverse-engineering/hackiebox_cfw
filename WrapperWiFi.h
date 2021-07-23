#ifndef WrapperWiFi_h
#define WrapperWiFi_h

#include <WiFi.h>

#include "BaseHeader.h"

#include <EnhancedThread.h>

class WrapperWiFi : public EnhancedThread {
  public:
    enum class ConnectionState {
      NONE,
      WAIT_CONNECT,
      WAIT_IP,
      CONNECTED,
      DISCONNECTED
    };

    WrapperWiFi() {};
    WrapperWiFi(const char* ssid, const char* password);
    WrapperWiFi(const char* ssid, const char* password, const byte ip[4], const byte subnet[4], const byte dns[4]);
    
    void
      begin(),
      loop();

    void
      reconnect();

    void apMode();

    WrapperWiFi::ConnectionState getStatus();

    void mDnsAdvertiseSetup();

  private:
    const char* _ssid;
    const char* _password;
    byte _ip[4];
    byte _subnet[4];
    byte _dns[4];

    ConnectionState _state = ConnectionState::NONE;
};

#endif