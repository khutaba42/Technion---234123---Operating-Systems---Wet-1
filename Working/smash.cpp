#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char *argv[])
{
    /**
     * change the signal handler for when the user clicks Ctrl+C
     * to use the function ctrlCHandler defined in signals.h
     */
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR)
    {
        perror("smash error: failed to set ctrl-C handler");
    }

    // TODO: setup sig alarm handler (bonus)
    /**
     * change the signal handler for when the process gets a SIG_ALRM signal
     * from the OS, after using the alarm() system call.
     * here we set the handler to the alarmHandler function defined in signals.h
     */
    if (sigaction(SIGALRM) == SIG_ERR)
    {
        perror("smash error: failed to set alarm handler");
    }
    

    // get the smash singleton instance locally
    SmallShell &smash = SmallShell::getInstance();
    // run an infinite loop for reading the next command for execution
    while (true)
    {
        // get the current prompt for the smash
        std::cout << smash.getPrompt() << "> ";
        // take in the command from the terminal
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        // execute the command
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}