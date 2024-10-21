// src/builtins.h
#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

bool isBuiltin(const Command& cmd);
void executeBuiltin(const Command& cmd);

#endif // BUILTINS_H
