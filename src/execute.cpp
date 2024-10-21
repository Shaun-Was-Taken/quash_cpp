// src/execute.cpp
#include "execute.h"
#include "parser.h"
#include "builtins.h"
#include "jobs.h"
#include <iostream>
#include <string>
#include <unistd.h>

void executeShell() {
    std::string input;
    while (true) {
        std::cout << "[QUASH]$ ";
        std::getline(std::cin, input);

        if (std::cin.eof()) {
            std::cout << "\n";
            break;
        }

        Command cmd = parseCommand(input);

        if (cmd.isEmpty()) {
            continue;
        }

        if (isBuiltin(cmd)) {
            executeBuiltin(cmd);
        } else {
            executeCommand(cmd);
        }

        checkBackgroundJobs();
    }
}
