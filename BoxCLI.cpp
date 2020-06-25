#include "BoxCLI.h"

void BoxCLI::begin() {
    setInterval(50);

    cmdHelp = cli.addCommand("help,h,?");
    cmdHelp.setDescription(" Get help!");
}

void BoxCLI::loop() {
    if (cli.available()) {
        Command c = cli.getCmd();
        Log.info("Received cmd: %s", c.toString().c_str());

        int argNum = c.countArgs();

        Log.print("> ");
        Log.print(c.getName());
        Log.print(' ');

        for (int i = 0; i<argNum; ++i) {
            Argument arg = c.getArgument(i);
            // if(arg.isSet()) {
            Log.print(arg.toString());
            Log.print(' ');
            // }
        }

        Log.println();

        if (c == cmdHelp) {
            Log.println("Help:");
            Log.println(cli.toString());
        }
    }

    if (cli.errored()) {
        CommandError cmdError = cli.getError();

        Log.print("ERROR: ");
        Log.println(cmdError.toString());

        if (cmdError.hasCommand()) {
            Log.print("Did you mean \"");
            Log.print(cmdError.getCommand().toString());
            Log.println("\"?");
        }
    }
}