#include "BoxDAC.h"

void BoxDAC::begin() { 
    Log.info("Initialize DAC...");
    //RESET
    pinMode(62, OUTPUT);
    digitalWrite(62, HIGH);
    delayMicroseconds(1); //Be sure 

    

    Log.info("...initialized");
}

void BoxDAC::loop() { 

}