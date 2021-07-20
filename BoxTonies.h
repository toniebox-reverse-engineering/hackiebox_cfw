#ifndef BoxTonies_h
#define BoxTonies_h

#include "BaseHeader.h"
#include "BoxSD.h"

class BoxTonies {
    public:
        bool
            loadTonieByUid(uint8_t uid[8]),
            loadTonieByPath(uint8_t* path);

        void logTonieHeader();

        struct TonieHeader {
            uint8_t hash[20];
            uint32_t audioLength; //length in bytes
            uint32_t audioId; //id, which is the unix time stamp of file creation
            uint32_t audioChapters[99]; //Ogg page numbers for Chapters
            uint8_t audioChapterCount;
        };
        
        uint8_t currentUid[8];
        
        const char* CONTENT_BASE = "/CONTENT/";
        const char* RCONTENT_BASE = "/rCONTENT/";

    private:
        FileFs tonieFile;

        TonieHeader header;

        uint64_t readVariant(uint8_t* buffer, uint16_t length, uint8_t& readBytes);
        void clearHeader();
        
};

#endif