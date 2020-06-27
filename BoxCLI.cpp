#include "BoxCLI.h"

#include "Hackiebox.h"

void BoxCLI::begin() {
    setInterval(50);

    cmdI2C = cli.addCmd("i2c");
    cmdI2C.setDescription("Access I2C");
    cmdI2C.addFlagArg("read");
    cmdI2C.addFlagArg("write");
    cmdI2C.addArg("a/ddress");
    cmdI2C.addArg("r/egister");
    cmdI2C.addArg("v/alue", "");
    cmdI2C.addArg("l/ength", "1");
    cmdI2C.addArg("o/utput", "b");

    cmdBeep = cli.addCmd("beep");
    cmdBeep.setDescription("Beep with build-in DAC synthesizer");
    cmdBeep.addArg("m/idi-id", "60");
    cmdBeep.addArg("l/ength", "200");

    cmdHelp = cli.addCmd("h/elp");
    cmdHelp.setDescription("Get help!");
}

void BoxCLI::loop() {
    parse();
}
void BoxCLI::parse() {
    if (cli.available()) {
        lastCmd = cli.getCmd();

        if (lastCmd == cmdHelp) {
            Log.println("Help:");
            Log.println(cli.toString().c_str());
        } else if (lastCmd == cmdI2C) {
            execI2C();
        } else if (lastCmd == cmdBeep) {
            execBeep();
        }
    }

    if (cli.errored()) {
        CommandError cmdError = cli.getError();

        Log.print("ERROR: ");
        Log.println(cmdError.toString().c_str());

        if (cmdError.hasCommand()) {
            Log.print("Did you mean \"");
            Log.print(cmdError.getCommand().toString().c_str());
            Log.println("\"?");
        }
    }
}

void BoxCLI::execI2C() {
    Command c = lastCmd;
    int argNum = c.countArgs();

    unsigned long int tmpNum;
    uint8_t addr, regi;

    /*
    Log.info("Received cmd: %s with %i arguments", c.toString().c_str(), argNum);
    for (int i = 0; i<argNum; ++i) {
        Argument arg = c.getArgument(i);
        // if(arg.isSet()) {
        Log.print(arg.toString().c_str());
        Log.print(" ");
        // }
    }
    */

    bool read = c.getArg("read").isSet();
    bool write = c.getArg("write").isSet();
    if (read == write) {
        Log.error("Either read or write has to be set");
        return;
    }

    String saddress = c.getArg("address").getValue();
    String sregister = c.getArg("register").getValue();

    tmpNum = parseNumber(saddress);
    if (tmpNum > 127) {
        Log.error("address must be lower than 128");
        return;
    }
    addr = (uint8_t)tmpNum;

    tmpNum = parseNumber(sregister);
    if (tmpNum > 255) {
        Log.error("register must be lower than 256");
        return;
    }
    regi = (uint8_t)tmpNum;

    //Log.printfln("I2C command read=%T, write=%T, addr=%X, regi=%X", read, write, addr, regi);
    String slength = c.getArg("length").getValue();
    String svalue = c.getArg("value").getValue();
    if (read) {
        if (slength.length() == 0) {
            Log.error("length must be specified");
            return;
        }
        tmpNum = parseNumber(slength);
        unsigned long len = tmpNum;
        
        Log.printfln("Read %i bytes", len);
        for (uint16_t i=0; i<len; i++) {
            uint8_t result = Box.boxI2C.readByte(addr, regi+i);
            Log.printf(" %X", result);
        }

    } else if (write) {
        if (svalue.length() == 0) {
            Log.error("value must be specified");
            return;
        }
        tmpNum = parseNumber(svalue);
        if (tmpNum > 255) {
            Log.error("register must be lower than 256");
            return;
        }
        uint8_t data = (uint8_t)tmpNum;
        bool result = Box.boxI2C.send(addr, regi, data);
        if (!result) {
            Log.error(" %X not written", data);
        }
        Log.printf(" %X successful written", data);
    }
}

void BoxCLI::execBeep() {
    Command c = lastCmd;
    unsigned long tmpNum;

    String sid = c.getArg("midi-id").getValue();
    tmpNum = parseNumber(sid);
    if (tmpNum > 127) {
        Log.error("midi-id must be lower than 128");
        return;
    }
    uint8_t id = (uint8_t)tmpNum;

    String slength = c.getArg("length").getValue();
    tmpNum = parseNumber(slength);
    if (tmpNum > 65535) {
        Log.error("length must be lower than 65.536");
        return;
    }
    Log.info("slength=%s, length=%l", slength.c_str(), tmpNum);
    uint16_t length = (uint16_t)tmpNum;

    Box.boxDAC.beepMidi(id, length);
}

unsigned long BoxCLI::parseNumber(String numberString) {
    const char* num = numberString.c_str();

    if (numberString.substring(0, 2) == "0x") {
        return strtoul(num, NULL, 16);
    } else if (numberString.substring(0, 2) == "0b") {
        return strtoul(num+2, NULL, 2);
    }

    return strtoul(num, NULL, 10);
}