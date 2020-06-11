#ifndef BoxRFID_h
#define BoxRFID_h

#include "BaseHeader.h"
#include <EnhancedThread.h>
#include <SPI.h>

class BoxRFID : public EnhancedThread { 
    public:
        void
            begin(),
            loop();
        
        void
            setSlaveSelect(bool enabled),
            sendCommand(int address, int value),
            sendCommand(int value);

};

#endif