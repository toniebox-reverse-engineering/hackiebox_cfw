
#include <Arduino.h>
#include <AudioLogger.h>

#include <Logging.h>

#ifndef _AUDIOCUSTOMLOGGER_H
#define _AUDIOCUSTOMLOGGER_H

class AudioCustomLogger: public DevNullOut
{
public:
    virtual size_t write(uint8_t c) {
        Log.printf("%c", c);
        return 1;
    };
    int printf_P(const char *msg, ...) {
        va_list args;
	    va_start(args, msg);
	    Log.printFormat(msg, args);
        return 0; //TOOD?
    };
};

#endif