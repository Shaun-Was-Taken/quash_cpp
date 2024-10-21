// src/builtins.cpp
#include "builtins.h"
#include "jobs.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <regex>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>

bool isBuiltin(const Command& cmd) {
    if (cmd.args.empty()) return false;
    std::string cmdName = cmd.args[0];
    return cmdName == "echo" || cmdName == "export" || cmdName == "cd" ||
           cmdName == "pwd" || cmdName == "exit" || cmdName == "quit" ||
           cmdName == "jobs";
}

void executeBuiltin(const Command& cmd) {
    // Save original file descriptors
    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);

    // Input redirection
    if (!cmd.inputRedirect.empty()) {
        int fd_in = open(cmd.inputRedirect.c_str(), O_RDONLY);
        if (fd_in < 0) {
            perror("open input");
            // Restore original file descriptors
            dup2(saved_stdin, STDIN_FILENO);
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdin);
            close(saved_stdout);
            return;
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    // Output redirection
    if (!cmd.outputRedirect.empty()) {
        int fd_out;
        if (cmd.appendOutput) {
            fd_out = open(cmd.outputRedirect.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        } else {
            fd_out = open(cmd.outputRedirect.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        if (fd_out < 0) {
            perror("open output");
            // Restore original file descriptors
            dup2(saved_stdin, STDIN_FILENO);
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdin);
            close(saved_stdout);
            return;
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    std::string cmdName = cmd.args[0];

    if (cmdName == "echo") {
        for (size_t i = 1; i < cmd.args.size(); ++i) {
            std::string arg = cmd.args[i];
            // Variable expansion
            size_t pos = 0;
            while ((pos = arg.find('$', pos)) != std::string::npos) {
                size_t end = pos + 1;
                while (end < arg.size() && (isalnum(arg[end]) || arg[end] == '_')) {
                    ++end;
                }
                std::string varName = arg.substr(pos + 1, end - pos - 1);
                const char* val = getenv(varName.c_str());
                if (val) {
                    arg.replace(pos, end - pos, val);
                    pos += strlen(val);
                } else {
                    arg.replace(pos, end - pos, "");
                }
            }
            std::cout << arg;
            if (i < cmd.args.size() - 1) {
                std::cout << " ";
            }
        }
        std::cout << "\n";
    } else if (cmdName == "export") {
        if (cmd.args.size() < 2) {
            std::cerr << "export: missing argument\n";
            // Restore original file descriptors
            dup2(saved_stdin, STDIN_FILENO);
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdin);
            close(saved_stdout);
            return;
        }
        for (size_t i = 1; i < cmd.args.size(); ++i) {
            std::string var = cmd.args[i];
            size_t pos = var.find('=');
            if (pos != std::string::npos) {
                std::string key = var.substr(0, pos);
                std::string value = var.substr(pos + 1);

                // Variable expansion in value
                size_t varPos = 0;
                while ((varPos = value.find('$', varPos)) != std::string::npos) {
                    size_t end = varPos + 1;
                    while (end < value.size() && (isalnum(value[end]) || value[end] == '_')) {
                        ++end;
                    }
                    std::string varName = value.substr(varPos + 1, end - varPos - 1);
                    const char* val = getenv(varName.c_str());
                    if (val) {
                        value.replace(varPos, end - varPos, val);
                        varPos += strlen(val);
                    } else {
                        value.replace(varPos, end - varPos, "");
                    }
                }

                setenv(key.c_str(), value.c_str(), 1);
            } else {
                std::cerr << "export: invalid format\n";
            }
        }
    } else if (cmdName == "cd") {
        const char* path = nullptr;
        if (cmd.args.size() > 1) {
            std::string dir = cmd.args[1];
            // Variable expansion
            if (!dir.empty() && dir[0] == '$') {
                const char* val = getenv(dir.substr(1).c_str());
                if (val) {
                    dir = val;
                } else {
                    dir = "";
                }
            }
            path = dir.c_str();
        } else {
            path = getenv("HOME");
        }

        if (chdir(path) != 0) {
            perror("cd");
        } else {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != nullptr) {
                setenv("PWD", cwd, 1);
            }
        }
    } else if (cmdName == "pwd") {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            std::cout << cwd << "\n";
        } else {
            perror("pwd");
        }
    } else if (cmdName == "exit" || cmdName == "quit") {
        // Restore original file descriptors before exiting
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdin);
        close(saved_stdout);
        exit(0);
    } else if (cmdName == "jobs") {
        listJobs();
    }

    // Restore original file descriptors
    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdin);
    close(saved_stdout);
}
