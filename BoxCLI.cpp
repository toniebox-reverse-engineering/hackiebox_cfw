#include "BoxCLI.h"

#include "Hackiebox.h"

void BoxCLI::begin() {
    setInterval(50);

    cmdI2C = cli.addCmd("i2c");
    cmdI2C.setDescription(" Access I2C");
    cmdI2C.addFlagArg("re/ad");
    cmdI2C.addFlagArg("wr/ite");
    cmdI2C.addArg("a/ddress");
    cmdI2C.addArg("r/egister");
    cmdI2C.addArg("v/alue", "");
    cmdI2C.addArg("l/ength", "1");
    cmdI2C.addArg("o/utput", "B");

    cmdRFID = cli.addCmd("rfid");
    cmdRFID.setDescription(" Access RFID SPI");
    cmdRFID.addFlagArg("re/ad");
    cmdRFID.addFlagArg("wr/ite");
    cmdRFID.addFlagArg("c/md,co/mmand");
    cmdRFID.addArg("r/egister", "0");
    cmdRFID.addArg("v/alue", "0");

    cmdBeep = cli.addCmd("beep");
    cmdBeep.setDescription(" Beep with build-in DAC synthesizer");
    cmdBeep.addArg("m/idi-id", "60");
    cmdBeep.addArg("l/ength", "200");

    cmdHelp = cli.addSingleArgumentCommand("help");
    cmdHelp.setDescription(" Show this screen");
}

void BoxCLI::loop() {
    parse();
}
void BoxCLI::parse() {
    if (cli.available()) {
        lastCmd = cli.getCmd();

        if (lastCmd == cmdHelp) {
            Log.println("Help:");
            Log.println();
            if (lastCmd.getArg(0).isSet()) {
                String arg = lastCmd.getArg(0).getValue();
                Command cmd = cli.getCmd(arg);
                if (cmd.getName() == arg) {
                    Log.println(cmd.toString().c_str());
                    return;
                }
            }
            
            Log.println(cli.toString().c_str());
        } else if (lastCmd == cmdI2C) {
            execI2C();
        } else if (lastCmd == cmdRFID) {
            execRFID();
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
        String soutput = c.getArg("output").getValue();
        if (!(soutput == "x" || soutput == "X" || soutput == "i" || soutput == "b" || soutput == "B")) {
            Log.error("Allowed values for output are: x, X, i, b, B");
            return;
        }

        if (slength.length() == 0) {
            Log.error("length must be specified");
            return;
        }
        tmpNum = parseNumber(slength);
        unsigned long len = tmpNum;
        
        Log.printfln("Read %i bytes", len);
        for (uint16_t i=0; i<len; i++) {
            String format = String(" %") + soutput;
            uint8_t result = Box.boxI2C.readByte(addr, regi+i);
            Log.printf(format.c_str(), result);
        }
    } else if (write) {
        char* value = (char*)svalue.c_str();
        char* newVal;
        while (strlen(value)>0) {
            while(isspace((unsigned char)*value)) value++;
            tmpNum = parseNumber(value, &newVal);
            if (tmpNum > 255) {
                Log.error("value must be lower than 256");
                return;
            } else if (value == newVal) {
                Log.error("Could not parse part \"%s\" of \"%s\"", value, svalue.c_str());
                return;
            }
            uint8_t data = (uint8_t)tmpNum;
            bool result = Box.boxI2C.send(addr, regi++, data);
            if (!result) {
                Log.error(" %X not written, abort", data);
                break;
            }
            Log.info(" %X successful written", data);
            value = newVal;
        }
    }
}

void BoxCLI::execRFID() {
    Command c = lastCmd;
    unsigned long tmpNum;

    String sregister = c.getArg("register").getValue();
    tmpNum = parseNumber(sregister);
    if (tmpNum > 255) {
        Log.error("register must be lower than 256");
        return;
    }
    uint8_t regi = (uint8_t)tmpNum;

    String svalue = c.getArg("value").getValue();
    tmpNum = parseNumber(svalue);
    if (tmpNum > 255) {
        Log.error("value/command must be lower than 256");
        return;
    }
    uint8_t value = (uint8_t)tmpNum;

    bool read = c.getArg("read").isSet();
    bool write = c.getArg("write").isSet();
    bool cmd = c.getArg("command").isSet();

    //TODO Exclusive check

    if (read) {
        Box.boxRFID.readRegister(regi);
    } else if (write) {
        Box.boxRFID.writeRegister(regi, value);
    } else if (cmd) {
        Box.boxRFID.sendCommand(value);
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
    uint16_t length = (uint16_t)tmpNum;

    Box.boxDAC.beepMidi(id, length);
}

unsigned long BoxCLI::parseNumber(String numberString) {
    const char* num = numberString.c_str();
    return parseNumber((char*)num);
}
unsigned long BoxCLI::parseNumber(char* number) {
    return parseNumber(number, NULL);
}
unsigned long BoxCLI::parseNumber(char* number, char** rest) {
    if (strncmp(number, "0x", 2) == 0) {
        return strtoul(number, rest, 16);
    } else if (strncmp(number, "0b", 2) == 0) {
        return strtoul(number+2, rest, 2);
    }
    return strtoul(number, rest, 10);
}