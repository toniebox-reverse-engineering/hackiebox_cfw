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
        Command cmdSpiRFID;
        Command cmdBeep;
        Command cmdRFID;
        Command cmdLoad;
        Command cmdI2S;
        Command cmdSay;
        Command cmdAudio;
        
        void parse();
        unsigned long
            parseNumber(String number),
            parseNumber(char* number),
            parseNumber(char* number, char** rest);

        void
            execI2C(),
            execSpiRFID(),
            execBeep(),
            execRFID(),
            execLoad(),
            execI2S(),
            execSay(),
            execAudio();
};

#endif