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
        Command lastCmd;

        Command cmdHelp;
        Command cmdI2C;
        Command cmdRFID;
        Command cmdBeep;
        
        void parse();
        unsigned long parseNumber(String number);

        void
            execI2C(),
            execRFID(),
            execBeep();
};

#endif