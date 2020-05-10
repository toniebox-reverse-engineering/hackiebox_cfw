#include "BoxSD.h"

void BoxSD::begin() { 
    Log.info("Initialize SD card");
    int result = FatFs.begin();
    if (!result) {
        Log.error("SD not mounted. Code %i", FatFs.error());
        return;
    }

    Log.info(" Capacity: %iMB", FatFs.capacity()/1024);
    Log.info(" Free: %iMB", FatFs.free()/1024);
}

void BoxSD::loop() { 

}

String BoxSD::jsonListDir(char* directory) {
    DynamicJsonDocument doc(1024);
    DirFs dir;
    
    String json;
    if (dir.openDir(directory)) {
        JsonArray files = doc.createNestedArray("files");
        while (dir.nextFile()) {
            JsonObject file = files.createNestedObject();
            file["name"] = dir.fileName();
            file["size"] = dir.fileSize();
            file["time"] = dir.fileModTime();
            file["date"] = dir.fileModDate();
            file["dir"] = dir.isDir();
        }
        dir.closeDir();
    } else {
        doc["error"] = "Directory not found";
        Log.error("Directory %s not found", directory);
    }
    serializeJson(doc, json);
    return json;
}