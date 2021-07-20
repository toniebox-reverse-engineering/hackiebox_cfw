#include "BoxConfig.h"

#define CONFIG_SD_PATH "/revvox/hackiebox.config.json"

void BoxConfig::begin() { 
    read();
}

void BoxConfig::read() {
    _initializeConfig();
    FileFs file;
    if (file.open(CONFIG_SD_PATH, FA_OPEN_EXISTING | FA_READ)) {
        char buffer[4096];
        size_t read;
        String json = String();
        while (file.curPosition() < file.fileSize()) {
            read = file.read(buffer, sizeof(buffer)); 
            if (read == 0)
                break;
            for (int i=0; i<read; i++) {
                json += buffer[i];
            }
        }
        file.close();
        if (!setFromJson(json)) {
            Log.error("Couldn't read cfg file %s, recreating it", CONFIG_SD_PATH);
            write();
        }
    } else {
        Log.error("Couldn't read cfg file %s, recreating it", CONFIG_SD_PATH);
        write();
    }
}
void BoxConfig::write() { 
    _json = getAsJson();
    FileFs file;
    if (file.open(CONFIG_SD_PATH, FA_CREATE_ALWAYS | FA_WRITE)) {
        file.writeString((char*)_json.c_str());
        file.close();
    } else {
        Log.error("Couldn't write cfg file %", CONFIG_SD_PATH);
    }
}

ConfigStruct* BoxConfig::get() { 
    return &_config;
}

String BoxConfig::getAsJson() { 
    StaticJsonDocument<BOXCONFIG_JSON_SIZE> doc;
    doc["version"] = _config.version;
    
    JsonObject batteryDoc = doc.createNestedObject("battery");
    ConfigBattery* batteryCfg = &_config.battery;
    batteryDoc["voltageFactor"] = batteryCfg->voltageFactor;
    batteryDoc["voltageChargerFactor"] = batteryCfg->voltageChargerFactor;
    batteryDoc["lowAdc"] = batteryCfg->lowAdc;
    batteryDoc["criticalAdc"] = batteryCfg->criticalAdc;
    batteryDoc["sleepMinutes"] = batteryCfg->sleepMinutes;

    JsonObject buttonsDoc = doc.createNestedObject("buttonEars");
    ConfigButtonEars* buttonCfg = &_config.buttonEars;
    buttonsDoc["longPressMs"] = buttonCfg->longPressMs;
    buttonsDoc["veryLongPressMs"] = buttonCfg->veryLongPressMs;

    JsonObject wifiDoc = doc.createNestedObject("wifi");
    ConfigWifi* wifiCfg = &_config.wifi;
    wifiDoc["ssid"] = wifiCfg->ssid;
    wifiDoc["password"] = wifiCfg->password;

    JsonObject logDoc = doc.createNestedObject("log");
    ConfigLog* logCfg = &_config.log;
    logDoc["sdLog"] = logCfg->sdLog;

    JsonObject miscDoc = doc.createNestedObject("misc");
    ConfigMisc* miscCfg = &_config.misc;
    miscDoc["autodump"] = miscCfg->autodump;

    serializeJson(doc, _json);
    return _json;
}
bool BoxConfig::setFromJson(String json) { 
    StaticJsonDocument<BOXCONFIG_JSON_SIZE> doc;

    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        Log.error("deserializeJson() returned %s, %s", err.c_str(), json.c_str());
        return false;
    }

    _config.version = doc["version"].as<uint8_t>();

    JsonObject batteryDoc = doc["battery"];
    ConfigBattery* batteryCfg = &_config.battery;
    batteryCfg->voltageFactor = batteryDoc["voltageFactor"].as<uint32_t>();
    batteryCfg->voltageChargerFactor = batteryDoc["voltageChargerFactor"].as<uint32_t>();
    batteryCfg->lowAdc = batteryDoc["lowAdc"].as<uint16_t>();
    batteryCfg->criticalAdc = batteryDoc["criticalAdc"].as<uint16_t>();
    batteryCfg->sleepMinutes = batteryDoc["sleepMinutes"].as<uint8_t>();

    JsonObject buttonsDoc = doc["buttonEars"];
    ConfigButtonEars* buttonCfg = &_config.buttonEars;
    buttonCfg->longPressMs = buttonsDoc["longPressMs"].as<uint16_t>();
    buttonCfg->veryLongPressMs = buttonsDoc["veryLongPressMs"].as<uint16_t>();

    JsonObject wifiDoc = doc["wifi"];
    ConfigWifi* wifiCfg = &_config.wifi;
    strncpy(&wifiCfg->ssid[0], wifiDoc["ssid"].as<char*>(), sizeof(wifiCfg->ssid));
    strncpy(&wifiCfg->password[0], wifiDoc["password"].as<char*>(), sizeof(wifiCfg->password));

    JsonObject logDoc = doc["log"];
    ConfigLog* logCfg = &_config.log;
    logCfg->sdLog = logDoc["sdLog"].as<bool>();

    JsonObject miscDoc = doc["misc"];
    ConfigMisc* miscCfg = &_config.misc;
    miscCfg->autodump = miscDoc["autodump"].as<bool>();

    // Convert old config version to latest one.
    if (_config.version != CONFIG_ACTIVE_VERSION) {
        switch (_config.version) {
        case 2:
            batteryCfg->criticalAdc = batteryDoc["minimalAdc"].as<uint16_t>();
            batteryCfg->lowAdc = batteryCfg->criticalAdc + 100;
            _config.version = 3;
            write();
            break;
        case 3:
            miscCfg->autodump = false;
            _config.version = 4;
            write();
            break;
        default:
            _initializeConfig();
            write();
            break;
        } 
    }

    return true;
}

void BoxConfig::_initializeConfig() { 
    _config.version = CONFIG_ACTIVE_VERSION;

    ConfigBattery* battery = &_config.battery;
    battery->voltageFactor = 67690;
    battery->voltageChargerFactor = 71907;
    battery->lowAdc = 2500;
    battery->criticalAdc = 2400;
    battery->sleepMinutes = 15;

    ConfigButtonEars* buttons = &_config.buttonEars;
    buttons->longPressMs = 1000;
    buttons->veryLongPressMs = 10000;

    ConfigWifi* wifi = &_config.wifi;
    wifi->dhcp = true;
    #ifdef WIFI_SSID
        strcpy(wifi->ssid, WIFI_SSID);
    #endif
    #ifdef WIFI_PASS
        strcpy(wifi->password, WIFI_PASS); 
    #endif

    ConfigLog* log = &_config.log;
    log->sdLog = false;

    ConfigMisc* misc = &_config.misc;
    misc->autodump = false;
}