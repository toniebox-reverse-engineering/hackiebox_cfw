#ifndef BoxSD_h
#define BoxSD_h

#include "BaseHeader.h"
#include <FatFs.h>

class BoxSD {
    public:
        void
            begin(),
            loop();

        String jsonListDir(char*);
    private:
};

#endif