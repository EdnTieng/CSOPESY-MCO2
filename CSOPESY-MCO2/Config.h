#pragma once
#include <iostream>

using namespace std;

extern int num_cpu;                // Number of CPUs
extern string sched;               // Scheduler type (fcfs or rr)
extern int quant_cycles;           // Quantum cycles range: 1 to 2^32
extern int batch_process_freq;     // Batch process frequency range: 1 to 2^32
extern int max_ins;                // Max instructions range: 1 to 2^32 
extern int min_ins;                // Min instructions range: 1 to 2^32 
extern int delay_per_exec;         // Delay per execution range: 0 to 2^32 

void assignConfig(int num_cpu_val, std::string sched_val, int quant_cycles_val, int batch_process_freq_val, int max_ins_val, int min_ins_val, int delay_per_exec_val);