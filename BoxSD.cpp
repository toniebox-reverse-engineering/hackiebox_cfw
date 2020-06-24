#include "BoxSD.h"

void BoxSD::begin() { 
    _initialized = false;
    Log.info("Initialize SD card");
    int result = FatFs.begin();
    if (!result) {
        Log.error("SD not mounted. Code %i", FatFs.error());
        return;
    }

    Log.info(" Capacity: %iMB", FatFs.capacity()/1024);
    Log.info(" Free: %iMB", FatFs.free()/1024);
    _initialized = true;
}

void BoxSD::loop() { 

}

bool BoxSD::isInitialized() {
    return _initialized;
}

void BoxSD::webJsonListDir(WebServer* webServer, char* directory) {
    DirFs dir; 
    if (dir.openDir(directory)) {
        webServer->sendContent("{\"files\":[");
        bool firstRound = true;
        while (dir.nextFile()) {
            StaticJsonDocument<361> file; //Maximum 256 chars filename length //https://arduinojson.org/v6/assistant/

            file["name"] = dir.fileName(); 
            file["size"] = dir.fileSize();
            file["time"] = dir.fileModTime();
            file["date"] = dir.fileModDate();
            file["dir"] = dir.isDir();

            size_t len = measureJson(file)+1;
            char json[len];
            serializeJson(file, json, len); //TODO directly stream to save mem
            if (!firstRound)
                webServer->sendContent(","); 
            webServer->sendContent(json); 
            firstRound = false;
        }
        dir.closeDir();
        webServer->sendContent("]}");
    } else {
        StaticJsonDocument<299> doc; //Maximum 256 chars path length //https://arduinojson.org/v6/assistant/
        doc["error"] = "Directory not found";
        Log.error("Directory %s not found", directory);

        size_t len = measureJson(doc)+1;
        char json[len];
        serializeJson(doc, json, len); //TODO directly stream to save mem
        webServer->sendContent(json); 
    }
}