#ifndef BoxTonies_h
#define BoxTonies_h

#include "BaseHeader.h"
#include "BoxSD.h"

class BoxTonies {
    public:
        void
            loadTonieByUid(uint8_t uid[8]),
            loadTonieByPath(uint8_t* path);

        struct TonieHeader {
            uint8_t hash;
            int64_t audioLength; //TODO check type len
            int64_t audioId; //TODO check type len
            uint64_t* audioChapters;//TODO check type len 
        };
        

    private:
        static const uint8_t* CONTENT_BASE = "/CONTENT/";
        FileFs tonieFile;
};

#endif