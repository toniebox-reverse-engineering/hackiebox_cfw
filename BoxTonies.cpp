#include "BoxTonies.h"

void BoxTonies::loadTonieByUid(uint8_t uid[8]) {
    uint8_t* path;

    asprintf(
        (char*)path,
        "%s%02X%02X%02X%02X/%02X%02X%02X%02X",
        CONTENT_BASE,
        tagUid[0], tagUid[1], tagUid[2], tagUid[3], tagUid[4], tagUid[5], tagUid[6], tagUid[7]
    );

    loadTonieByPath(path);
}

void BoxTonies::loadTonieByPath(uint8_t* path) {
    Log.info("Loading Tonie from path %s...", path);
    
    if (tonieFile.open((const char*)path, FA_OPEN_EXISTING | FA_READ)) {
        uint8_t buffer[512]; //TODO: buffer >512 size may scramble the stream 4096 block needed
        read = file.read(buffer, sizeof(buffer));
        if (read > 0) {
            uint16_t cursor = 0;
            uint16_t bufferLen = 0;

            while (cursor < bufferLen-1) {
                uint8_t fieldId = buffer[cursor]>>3;
                uint8_t fieldType = buffer[cursor]&0b00000111;
                cursor++;

                uint8_t start = cursor;
                switch (fieldId) {
                case 0: //Variant
                    break;
                case 1: //Fixed64
                    break;
                case 2: //Length-delimited
                    break;
                case 5: //Fixed32
                    break;
                
                default:
                    break;
                }
            }
        } else {
            Log.error("... could not data from file.");
        }
        file.close();
    } else {
        Log.error("... could not open Tonie.");
    }
}