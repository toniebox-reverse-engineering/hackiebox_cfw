#include "BoxCLI.h"

void BoxCLI::begin() {
    setInterval(50);

    cmdI2C = cli.addCmd("i2c");
    cmdI2C.setDescription("Access I2C");
    cmdI2C.addFlagArg("r/ead");
    cmdI2C.addFlagArg("w/rite");
    cmdI2C.addArg("a/ddress");
    cmdI2C.addArg("r/egister");
    cmdI2C.addArg("v/alue", "");
    cmdI2C.addArg("l/ength", "");
    cmdI2C.addArg("o/utput", "b");

    cmdHelp = cli.addCmd("h/elp");
    cmdHelp.setDescription("Get help!");
}

void BoxCLI::loop() {
    if (cli.available()) {
        Command c = cli.getCmd();
        int argNum = c.countArgs();

        Log.println();

        if (c == cmdHelp) {
            Log.println("Help:");
            Log.println(cli.toString().c_str());
        } else if (c == cmdI2C) {
            unsigned long int tmpNum;
            uint8_t addr, regi;

            int argNum = c.countArgs();

            Log.info("Received cmd: %s with %i arguments", c.toString().c_str(), argNum);

            for (int i = 0; i<argNum; ++i) {
                Argument arg = c.getArgument(i);
                // if(arg.isSet()) {
                Log.print(arg.toString().c_str());
                Log.print(" ");
                // }
            }


            bool read = c.getArg("read").isSet();
            bool write = c.getArg("write").isSet();
            if (read == write) {
                Log.error("Either read or write has to be set");
                return;
            }

            String saddress = c.getArg("address").toString();
            String sregister = c.getArg("register").toString();

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

            String slength = c.getArg("length").toString();
            String svalue = c.getArg("value").toString();
            if (read) {
                if (slength.length() == 0) {
                    Log.error("length must be specified");
                    return;
                }
            } else if (write) {
                if (svalue.length() == 0) {
                    Log.error("value must be specified");
                    return;
                }
            }
            Log.info("I2C command read=%T, write=%T, addr=%i, regi=%i", read, write, addr, regi);
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

unsigned long int BoxCLI::parseNumber(String numberString) {
    const char* num = numberString.c_str();

    if (numberString.substring(2) == "0x") {
        num += 2;
        return strtoul(num, NULL, 16);
    } else if (numberString.substring(2) == "0b") {
        num += 2;
        return strtoul(num, NULL, 2);
    }

    return strtoul(num, NULL, 10);
}