#ifndef BoxCLI_h
#define BoxCLI_h

#include "BaseHeader.h"
#include <SimpleCLI.h>
#include <EnhancedThread.h>

class BoxCLI : public EnhancedThread {
    public:
        void
            begin(),
            loop();

        SimpleCLI cli;

    private:
        Command cmdHelp;
        Command cmdI2C;
        
        void parse();
        unsigned long parseNumber(String number);
};

#endif