#pragma once
#ifndef FCFS_H
#define FCFS_H

#include "Process.h" // Include Process header
#include "consoleManager.h"
#include "MemoryManager.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <unordered_map>

using namespace std;

// Enum to define the scheduling algorithm type
enum SchedulingAlgorithm {
    FCFS,
    RR
};

class FCFS_Scheduler {
public:
    FCFS_Scheduler(int coreCount, ConsoleManager* consoleManager);

    void start();
    void schedulingTestStart(bool run);
    void stop();
    void addToQueue(string name);
    void vmstat();

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

    MemoryManager memoryManager;
    ConsoleManager* consoleManager; // Reference to ConsoleManager

    SchedulingAlgorithm algorithm;  // Store the selected scheduling algorithm

    void schedulerFunction();
    void cpuWorker(int coreId);
    
    mutex statsMutex; // Protects tick data
    vector<int> coreIdleTicks;   // Tracks idle ticks for each core
    vector<int> coreActiveTicks; // Tracks active ticks for each core
};

#endif // FCFS_H
