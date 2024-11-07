#pragma once
#include <iostream>

using namespace std;

extern int num_cpu;					// Number of CPUs
extern string sched;				// Scheduler type (fcfs or rr)
extern int quant_cycles;			// Quantum cycles range: 1 to 2^32
extern int batch_process_freq;		// Batch process frequency range: 1 to 2^32
extern int max_ins;					// Max instructions range: 1 to 2^32 
extern int min_ins;					// Min instructions range: 1 to 2^32 
extern int delay_per_exec;			// Delay per execution range: 0 to 2^32
extern int max_overall_mem;			// Range: 2 to 2^32 power of 2 format
extern int mem_per_frame;			// Range: 2 to 2^32 power of 2 format
extern int min_mem_per_proc;		// Range: 2 to 2^32 power of 2 format
extern int max_mem_per_proc;		// Range: 2 to 2^32 power of 2 format

void assignConfig(int num_cpu_val, std::string sched_val, int quant_cycles_val, int batch_process_freq_val, int max_ins_val, int min_ins_val, int delay_per_exec_val);

void assignConfig2(int max_overall_mem_val, int mem_per_frame_val, int min_mem_per_proc_val, int max_mem_per_proc_val);