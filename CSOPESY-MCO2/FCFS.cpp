#include "FCFS.h"
#include "Config.h"  // Assuming this includes sched_algo and quantum_cycles
#include <iomanip>    // For put_time
#include <sstream>    // For ostringstream
#include <random>     // For random number generation


using namespace std;

FCFS_Scheduler::FCFS_Scheduler(int coreCount, ConsoleManager* consoleManager)
    : coreCount(coreCount), running(false), consoleManager(consoleManager) {

    coreAvailable.resize(coreCount, true);

    // Determine the scheduling algorithm based on the global `sched_algo` variable
    if (sched == "fcfs") {
        algorithm = FCFS;
    }
    else if (sched == "rr") {
        algorithm = RR;
    }
    else {
        throw std::invalid_argument("Invalid scheduling algorithm specified in Config.h");
    }
}

void FCFS_Scheduler::start() {
    running = true;

    // Start the scheduler thread
    schedulerThread = thread(&FCFS_Scheduler::schedulerFunction, this);

    // Start the CPU worker threads
    for (int i = 0; i < coreCount; i++) {
        cpuWorkers.emplace_back(&FCFS_Scheduler::cpuWorker, this, i);
    }
}


void FCFS_Scheduler::schedulingTestStart(bool run) {
    if (run) {
        testRunning = true;  // Start the scheduling test loop
        schedulingTestThread = std::thread([this]() {
            int processId = 0;
            while (testRunning) {  // Use testRunning to control this loop
                // Create and enqueue a new process
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    processQueue.push(new Process(processId++,max_mem_per_proc));
                }

                // Notify worker threads about the new process
                cv.notify_all();

                // Delay between process creations, adjust as needed
                std::this_thread::sleep_for(std::chrono::milliseconds(batch_process_freq * 10));
            }
            });
    }
    else {
        // Stop the scheduling test loop without affecting the main scheduler
        testRunning = false;
        if (schedulingTestThread.joinable()) {
            schedulingTestThread.join();
        }
    }
}


void FCFS_Scheduler::stop() {
    running = false; // Signal all threads to stop

    // Notify all workers to unblock any waiting threads
    cv.notify_all();

    // Join the scheduler thread if it's active
    cout << "none check\n";
    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }
    cout << "sched check\n";
    // Join the CPU worker threads
    for (auto& worker : cpuWorkers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    cout << "cpu check\n";
    // Join the scheduling test thread if it’s active
    if (schedulingTestThread.joinable()) {
        schedulingTestThread.join();
    }
    cout << "schedTest check\n";
}


void FCFS_Scheduler::schedulerFunction() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (!running) {
            // Exit if scheduler is stopped
            break;
        }

        if (!processQueue.empty()) {
            // Notify workers when there are processes
            cv.notify_all();
        }
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust timing if needed
    }
}

void FCFS_Scheduler::cpuWorker(int coreId) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min_ins, max_ins);
    int qq = 0;
    while (true) {
        Process* process = nullptr;

        {
            std::unique_lock<std::mutex> lock(queueMutex);

            // Wait for an available process or if the scheduler is stopping
            cv.wait(lock, [this, coreId] {
                return (!processQueue.empty() && coreAvailable[coreId]) || !running;
                });

            if (!running && processQueue.empty()) break;

            if (!processQueue.empty() && coreAvailable[coreId]) {
                Process* nextProcess = processQueue.front();
                
                // Check if memory is available for the next process
                if (currentMemoryUsage + max_mem_per_proc <= max_overall_mem+1 || nextProcess->in_mem == true) {
                    // Assign process to core and allocate memory
                    if (nextProcess->in_mem == false)
                    {
                        currentMemoryUsage = currentMemoryUsage + max_mem_per_proc;
                        nextProcess->in_mem = true;
                    }
                    process = nextProcess;
                    processQueue.pop();
                    
                    coreAvailable[coreId] = false;
                    coreAssignments[coreId] = process;

                    // Assign random instructions if not set
                    if (process->total_ins == 0) {
                        process->total_ins = dist(gen);
                    }

                    // Timestamp for new process
                    auto now = std::time(nullptr);
                    struct tm local_time;
                    localtime_s(&local_time, &now);
                    std::ostringstream oss;
                    oss << std::put_time(&local_time, "%m/%d/%Y %I:%M:%S %p");
                    std::string timestamp = oss.str();

                    if (process->dummy) {
                        consoleManager->addProcess("P" + std::to_string(process->id), "Running", coreId, timestamp, process->current_ins, process->total_ins, process->mem_allocated, true);
                    }
                    else {
                        consoleManager->addProcess(process->name, "Running", coreId, timestamp, process->current_ins, process->total_ins, process->mem_allocated, true);
                    }
                }
                else {
                    // Not enough memory; send process back to the queue tail
                    processQueue.push(processQueue.front());
                    processQueue.pop();
                }
            }
        }

        // Execute the assigned process
        if (process) {
            int instructions_to_execute = (algorithm == RR) ? std::min(quant_cycles, process->total_ins - process->current_ins) : process->total_ins;

            for (int i = 0; i < instructions_to_execute; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_per_exec * 10));
                qq++; // Increment qq
                process->current_ins++;
                
                // Log memory status after each instruction execution
                //consoleManager->printMemoryStamp(qq, max_overall_mem, currentMemoryUsage);

                if (process->dummy) {
                    consoleManager->updateProcessStatus("P" + std::to_string(process->id), "Running", process->current_ins, true);
                }
                else {
                    consoleManager->updateProcessStatus(process->name, "Running", process->current_ins, true);
                }
            }

            {
                std::lock_guard<std::mutex> lock(queueMutex);

                // Release memory if the process is finished
                if (process->current_ins >= process->total_ins) {
                    if (process->dummy) {
                        consoleManager->updateProcessStatus("P" + std::to_string(process->id), "Finished", process->total_ins, false);
                    }
                    else {
                        consoleManager->updateProcessStatus(process->name, "Finished", process->total_ins, false);
                    }
                    delete process;
                    currentMemoryUsage -= max_mem_per_proc;
                    coreAvailable[coreId] = true;
                    coreAssignments.erase(coreId);
                }
                else if (algorithm == RR) {
                    // Round Robin: Re-queue if the process has remaining instructions
                    if (process->dummy) {
                        consoleManager->updateProcessStatus("P" + std::to_string(process->id), "Waiting", process->current_ins, true);
                    }
                    else {
                        consoleManager->updateProcessStatus(process->name, "Waiting", process->current_ins, true);
                    }

                    processQueue.push(process);
                    coreAvailable[coreId] = true;
                    coreAssignments.erase(coreId);
                    cv.notify_all(); // Notify other threads
                }
            }
        }
    }
}




void FCFS_Scheduler::addToQueue(string name) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        processQueue.push(new Process(name, max_mem_per_proc));
    }
    cv.notify_all();
}
