#pragma once
#ifndef FCFS_H
#define FCFS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <string>
#include "consoleManager.h"
#include <unordered_map>
#include <vector>

using namespace std;

// Enum to define the scheduling algorithm type
enum SchedulingAlgorithm {
    FCFS,
    RR
};

class Process {
public:
    int id;
    bool dummy;
    int total_ins = 0;   // Total instructions for the process
    int current_ins = 0; // Track executed instructions for RR
    int mem_allocated = 0;
    bool in_mem;
    string name;

    Process(int pid,int mem_aloc) : id(pid), mem_allocated(mem_aloc), dummy(true), in_mem(false) {}
    Process(string name, int mem_aloc) : name(name), mem_allocated(mem_aloc), dummy(false), in_mem(false) {}
};

class FCFS_Scheduler {
public:
    FCFS_Scheduler(int coreCount, ConsoleManager* consoleManager);

    void start();
    void schedulingTestStart(bool run);
    void stop();
    void addToQueue(string name);

private:
    int coreCount;
    int currentMemoryUsage = 0;
    atomic<bool> running;
    atomic<bool> testRunning;
    vector<thread> cpuWorkers;
    thread schedulerThread;
    thread schedulingTestThread;
    queue<Process*> processQueue;
    mutex queueMutex;
    condition_variable cv;
    vector<bool> coreAvailable; // Track core availability
    unordered_map<int, Process*> coreAssignments; // Map to track core assignments

    ConsoleManager* consoleManager; // Reference to ConsoleManager

    SchedulingAlgorithm algorithm;  // Store the selected scheduling algorithm

    void schedulerFunction();
    void cpuWorker(int coreId);
};

#endif // FCFS_H
