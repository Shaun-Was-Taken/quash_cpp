// src/parser.cpp
#include "parser.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

Command::Command()
    : appendOutput(false), background(false), pipe(false), next(nullptr) {}

bool Command::isEmpty() {
    return args.empty() && !background && !pipe && inputRedirect.empty() && outputRedirect.empty();
}

std::vector<std::string> tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::string token;
    bool inQuotes = false;
    char quoteChar = '\0';
    size_t i = 0;

    while (i < input.length()) {
        char c = input[i];

        if (c == '#' && !inQuotes) {
            // Ignore rest of line as comment
            break;
        }

        if (isspace(c) && !inQuotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            ++i;
        } else if (c == '\'' || c == '\"') {
            if (inQuotes && c == quoteChar) {
                inQuotes = false;
                quoteChar = '\0';
                ++i;
            } else if (!inQuotes) {
                inQuotes = true;
                quoteChar = c;
                ++i;
            } else {
                token += c;
                ++i;
            }
        } else {
            token += c;
            ++i;
        }
    }

    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

Command parseCommand(const std::string& input) {
    Command cmd;
    std::vector<std::string> tokens = tokenize(input);
    bool redirectOut = false;
    bool redirectIn = false;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];

        if (token == ">") {
            redirectOut = true;
            cmd.appendOutput = false;
        } else if (token == ">>") {
            redirectOut = true;
            cmd.appendOutput = true;
        } else if (token == "<") {
            redirectIn = true;
        } else if (token == "|") {
            cmd.pipe = true;
            // Collect the rest of the tokens and parse as next command
            std::string rest;
            for (size_t j = i + 1; j < tokens.size(); ++j) {
                rest += tokens[j] + " ";
            }
            cmd.next = new Command(parseCommand(rest));
            break;
        } else if (token == "&") {
            cmd.background = true;
        } else if (redirectOut) {
            cmd.outputRedirect = token;
            redirectOut = false;
        } else if (redirectIn) {
            cmd.inputRedirect = token;
            redirectIn = false;
        } else {
            cmd.args.push_back(token);
        }
    }

    return cmd;
}
