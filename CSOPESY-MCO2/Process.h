#pragma once
#ifndef PROCESS_H
#define PROCESS_H

#include <string>
using namespace std;

class Process {
public:
    int id;
    bool dummy;
    int total_ins = 0;   // Total instructions for the process
    int current_ins = 0; // Track executed instructions
    int mem_allocated = 0;
    bool in_mem;
    std::string name;

    // Constructors
    Process(int pid, int mem_aloc);
    Process(const std::string& name, int mem_aloc);
    Process(int id, std:: string name, int currentIns, int totalIns, int memAllocated);
};

#endif // PROCESS_H
