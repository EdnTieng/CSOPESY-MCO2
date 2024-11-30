#pragma once
#ifndef CONSOLE_MANAGER_H
#define CONSOLE_MANAGER_H

#include <vector>
#include <string>

// Process info struct to store process information
struct ProcessInfo {
    std::string name;       // Process name
    std::string status;     // "Running" or "Finished"
    int coreId;             // Core ID the process is assigned to
    std::string timestamp;  // Start timestamp
    int progress;           // Progress percentage (0-100)
    int num_ins;
    int mem_allocated;
    bool in_mem;
};

// ConsoleManager class to manage and display process information
class ConsoleManager {
public:
    // Adds a new process with its name, status, core ID, timestamp, and initial progress
    void addProcess(const std::string& process_name, const std::string& status, int coreId, const std::string& timestamp, int progress, int num_ins, int mem_allocated, bool in_mem);

    // Updates the status and progress of an existing process
    void updateProcessStatus(const std::string& process_name, const std::string& status, int progress, bool in_mem);

    // Lists all processes, displaying both "Running" and "Finished" processes
    void listProcesses() const;

    // Displays details of a specific process by name
    void displayProcess(const std::string& process_name) const;

    // Removes a process from the list by name
    void removeProcess(const std::string& process_name);

    //report-util
    void printProcesses() const;

    //memory stamp
    void printMemoryStamp(int qq, int maxOverallMem, int currentMemoryUsage) const;

    //process-smi
    void process_smi() const;

    //vmstat
    void vmstat() const;

    int getPageOuts(int pageOutsFromMem) const;

    int getPageIns(int pageInsFromMem) const;
    int pageIns;
    int pageOuts;
private:
    std::vector<ProcessInfo> processes; // Vector to store all processes

};

#endif // CONSOLE_MANAGER_H
