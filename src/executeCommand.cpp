// src/executeCommand.cpp
#include "execute.h"
#include "jobs.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <cstring>

void executeCommand(Command& cmd) {
    pid_t pid = fork();

    if (pid == 0) { // Child process
        if (!cmd.inputRedirect.empty()) {
            int fd = open(cmd.inputRedirect.c_str(), O_RDONLY);
            if (fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (!cmd.outputRedirect.empty()) {
            int fd;
            if (cmd.appendOutput) {
                fd = open(cmd.outputRedirect.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                fd = open(cmd.outputRedirect.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if (fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        if (cmd.pipe && cmd.next) {
            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }
            pid_t pid2 = fork();
            if (pid2 == 0) { // Left side of pipe
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
                execvp(cmd.args[0].c_str(), const_cast<char* const*>(convertArgs(cmd.args)));
                perror("execvp");
                exit(1);
            } else { // Right side of pipe
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
                execvp(cmd.next->args[0].c_str(), const_cast<char* const*>(convertArgs(cmd.next->args)));
                perror("execvp");
                exit(1);
            }
        } else {
            execvp(cmd.args[0].c_str(), const_cast<char* const*>(convertArgs(cmd.args)));
            perror("execvp");
            exit(1);
        }
    } else if (pid > 0) { // Parent process
        if (cmd.background) {
            addJob(pid, reconstructCommand(cmd));
        } else {
            waitpid(pid, nullptr, 0);
        }
    } else {
        perror("fork");
    }
}

char** convertArgs(const std::vector<std::string>& args) {
    char** argv = new char*[args.size() + 1];
    for (size_t i = 0; i < args.size(); ++i) {
        argv[i] = strdup(args[i].c_str());
    }
    argv[args.size()] = nullptr;
    return argv;
}

std::string reconstructCommand(const Command& cmd) {
    std::string command;
    for (const auto& arg : cmd.args) {
        command += arg + " ";
    }
    if (cmd.background) {
        command += "&";
    }
    return command;
}
