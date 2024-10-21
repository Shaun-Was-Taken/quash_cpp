// src/jobs.cpp
#include "jobs.h"
#include <sys/wait.h>
#include <iostream>

static std::vector<Job> jobs;
static int nextJobId = 1;

void addJob(pid_t pid, const std::string& command) {
    Job job = {nextJobId++, pid, command};
    jobs.push_back(job);
    std::cout << "Background job started: [" << job.jobId << "] " << job.pid
              << " " << job.command << "\n";
}

void removeJob(pid_t pid) {
    for (auto it = jobs.begin(); it != jobs.end(); ++it) {
        if (it->pid == pid) {
            std::cout << "Completed: [" << it->jobId << "] " << it->pid << " "
                      << it->command << "\n";
            jobs.erase(it);
            break;
        }
    }
}

void listJobs() {
    for (const auto& job : jobs) {
        std::cout << "[" << job.jobId << "] " << job.pid << " "
                  << job.command << " &\n";
    }
}

void checkBackgroundJobs() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        removeJob(pid);
    }
}
