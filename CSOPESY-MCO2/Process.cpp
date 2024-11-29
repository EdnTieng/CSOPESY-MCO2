#include "Process.h"
#include <iostream>

// Constructor for dummy processes
Process::Process(int pid, int mem_aloc)
    : id(pid), name("P"+to_string(pid)), mem_allocated(mem_aloc), dummy(true), in_mem(false) {}

// Constructor for named processes
Process::Process(const std::string& name, int mem_aloc)
    : name(name), mem_allocated(mem_aloc), dummy(false), in_mem(false) {}

Process::Process(int id, std::string name, int currentIns, int totalIns, int memAllocated)
    : id(id), name(name), current_ins(currentIns), total_ins(totalIns), mem_allocated(memAllocated), in_mem(false){}
