#include "Config.h"
#include <iostream>


int num_cpu;                // Number of CPUs
string sched;               // Scheduler type (fcfs or rr)
int quant_cycles;           // Quantum cycles range: 1 to 2^32
int batch_process_freq;     // Batch process frequency range: 1 to 2^32
int max_ins;                // Max instructions range: 1 to 2^32 
int min_ins;                // Min instructions range: 1 to 2^32 
int delay_per_exec;         // Delay per execution range: 0 to 2^32 

void assignConfig(int num_cpu_val, std::string sched_val, int quant_cycles_val, int batch_process_freq_val, int min_ins_val, int max_ins_val, int delay_per_exec_val)
{
    ::num_cpu = num_cpu_val;
    ::sched = sched_val;
    ::quant_cycles = quant_cycles_val;
    ::batch_process_freq = batch_process_freq_val;
    ::max_ins = max_ins_val;
    ::min_ins = min_ins_val;
    ::delay_per_exec = delay_per_exec_val;
}
