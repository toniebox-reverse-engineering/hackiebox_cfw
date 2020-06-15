#ifndef BoxDAC_h
#define BoxDAC_h

#include "BaseHeader.h"
#include <EnhancedThread.h>

class BoxDAC : public EnhancedThread  { 
    public:
        void
            begin(),
            loop();
};

#endif