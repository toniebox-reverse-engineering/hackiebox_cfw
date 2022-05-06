#ifndef ConfigStructures_h
#define ConfigStructures_h

#define CONFIG_ACTIVE_VERSION 7

typedef struct {
  uint32_t voltageFactor;
  uint32_t _deprecated_voltageChargerFactor;
  uint16_t lowAdc;
  uint16_t criticalAdc;
  uint8_t sleepMinutes;
} ConfigBattery;

typedef struct {
  uint16_t longPressMs;
  uint16_t veryLongPressMs;
} ConfigButtonEars;

typedef struct {
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;  
} ConfigIP;

typedef struct {
  char ssid[32];
  char password[64];
  
  bool dhcp;
  ConfigIP ip;
  ConfigIP subnet;
  ConfigIP dns;
  
  char hostname[32];
} ConfigWifi;

typedef struct {
  bool sdLog;
} ConfigLog;

typedef struct {
  bool autodump;
  bool swd;
  uint8_t watchdogSeconds;
} ConfigMisc;

typedef struct {
  uint8_t version;
  ConfigBattery battery;
  ConfigButtonEars buttonEars;
  ConfigWifi wifi;
  ConfigLog log;
  ConfigMisc misc;
} ConfigStruct;

#endif