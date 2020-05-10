#include "BoxConfig.h"

void BoxConfig::begin() { 
    _initializeConfig();
    read();
}

void BoxConfig::read() {

}
void BoxConfig::write() { 

}

ConfigStruct* BoxConfig::get() { 
    return &_config;
}

String BoxConfig::getAsJson() { 
    DynamicJsonDocument doc(BOXCONFIG_JSON_SIZE);
    String json;

    doc["version"] = CONFIG_ACTIVE_VERSION;
    
    JsonObject batteryDoc = doc.createNestedObject("battery");
    ConfigBattery* batteryCfg = &_config.battery;
    batteryDoc["voltageFactor"] = batteryCfg->voltageFactor;
    batteryDoc["voltageChargerFactor"] = batteryCfg->voltageChargerFactor;
    batteryDoc["minimalAdc"] = batteryCfg->minimalAdc;
    batteryDoc["sleepMinutes"] = batteryCfg->sleepMinutes;

    JsonObject buttonsDoc = doc.createNestedObject("buttonEars");
    ConfigButtonEars* buttonCfg = &_config.buttonEars;
    buttonsDoc["longPressMs"] = buttonCfg->longPressMs;
    buttonsDoc["veryLongPressMs"] = buttonCfg->veryLongPressMs;

    serializeJson(doc, json);
    return json;
}
void BoxConfig::setFromJson(String json) { 
    DynamicJsonDocument doc(BOXCONFIG_JSON_SIZE);

    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        Log.error("deserializeJson() returned %s", err.c_str());
        return;
    }

    JsonObject batteryDoc = doc["battery"];
    ConfigBattery* batteryCfg = &_config.battery;
    batteryCfg->voltageFactor = batteryDoc["voltageFactor"];
    batteryCfg->voltageChargerFactor = batteryDoc["voltageChargerFactor"];
    batteryCfg->minimalAdc = batteryDoc["minimalAdc"];
    batteryCfg->sleepMinutes = batteryDoc["sleepMinutes"];

    JsonObject buttonsDoc = doc["buttonEars"];
    ConfigButtonEars* buttonCfg = &_config.buttonEars;
    buttonCfg->longPressMs = buttonsDoc["longPressMs"];
    buttonCfg->veryLongPressMs = buttonsDoc["veryLongPressMs"];
}

void BoxConfig::_initializeConfig() { 
    _config.version = CONFIG_ACTIVE_VERSION;

    //Battery
    _config.battery.voltageFactor = 67690;
    _config.battery.voltageChargerFactor = 71907;
    _config.battery.minimalAdc = 2400;
    _config.battery.sleepMinutes = 15;

    //Button Ears
    _config.buttonEars.longPressMs = 1000;
    _config.buttonEars.veryLongPressMs = 10000;
}