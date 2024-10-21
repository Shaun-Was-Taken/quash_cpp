// src/jobs.h
#ifndef JOBS_H
#define JOBS_H

#include "parser.h"
#include <vector>
#include <string>

struct Job {
    int jobId;
    pid_t pid;
    std::string command;
};

void addJob(pid_t pid, const std::string& command);
void removeJob(pid_t pid);
void listJobs();
void checkBackgroundJobs();

#endif // JOBS_H
