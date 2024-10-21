// src/parser.h
#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

struct Command {
    std::vector<std::string> args;
    std::string inputRedirect;
    std::string outputRedirect;
    bool appendOutput;
    bool background;
    bool pipe;
    Command* next;

    Command();
    bool isEmpty();
};

Command parseCommand(const std::string& input);
std::vector<std::string> tokenize(const std::string& input);

#endif // PARSER_H
