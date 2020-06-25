#ifndef BoxCLI_h
#define BoxCLI_h

#include "BaseHeader.h"
#include <SimpleCLI.h>

class BoxCLI {
    public:
        void
            begin(),
            loop();

        SimpleCLI cli;

    private:
        Command cmdHelp;
};

#endif