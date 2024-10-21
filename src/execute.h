// src/execute.h
#ifndef EXECUTE_H
#define EXECUTE_H

#include "parser.h"

void executeShell();
void executeCommand(Command& cmd);
char** convertArgs(const std::vector<std::string>& args);
std::string reconstructCommand(const Command& cmd);

#endif // EXECUTE_H
