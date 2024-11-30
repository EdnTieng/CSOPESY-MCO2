#include "FCFS.h"
#include "header.h"
#include "Process.h"
#include "Config.h"
#include <iomanip>
#include <sstream>
#include <random>
#include <ctime>
using namespace std;


using namespace std;

FCFS_Scheduler::FCFS_Scheduler(int coreCount, ConsoleManager* consoleManager)
    : coreCount(coreCount),
    running(false),
    consoleManager(consoleManager),
    memoryManager(max_overall_mem, mem_per_frame) { // Initialize MemoryManager
    coreIdleTicks.resize(coreCount, 0);   // Set all cores' idle ticks to 0
    coreActiveTicks.resize(coreCount, 0); // Set all cores' active ticks to 0
    coreAvailable.resize(coreCount, true);

    // Determine the scheduling algorithm
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
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(min_mem_per_proc, max_mem_per_proc);
            int processId = 0;
            while (testRunning) {  // Use testRunning to control this loop
                // Create and enqueue a new process
                {
                    int randomized_mem = dist(gen);
                    std::lock_guard<std::mutex> lock(queueMutex);
                    processQueue.push(new Process(processId++, randomized_mem));
                    
                }

                // Notify worker threads about the new process
                cv.notify_all();

                // Delay between process creations, adjust as needed
                std::this_thread::sleep_for(std::chrono::milliseconds(batch_process_freq +1 * 10));
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

        // Check if memory is available to load processes from the backing store
        if (memoryManager.getUsedBlocks() < memoryManager.getTotalBlocks() && !memoryManager.isBackingStoreEmpty()) {
            // Retrieve the next process ID from the backing store
            int nextProcessId = memoryManager.peekNextProcessIdFromStore();  // Get the next process ID from the backing store

            if (nextProcessId != -1) {  // If a valid ID is returned, proceed

                // Check if memory is available to load this new process
                int requiredBlocks = max_mem_per_proc / mem_per_frame;
                if (memoryManager.getUsedBlocks() + requiredBlocks > memoryManager.getTotalBlocks()) {
                    // If memory is not enough, remove the oldest process and store it in the backing store
                    Process* oldestProcess = processQueue.front();
                    processQueue.pop();  // Remove the oldest process from the queue
                    memoryManager.writeProcessToStore(oldestProcess);  // Store it in the backing store
                }

                // Now we can safely load the new process into memory
                Process* process = memoryManager.readProcessFromStore(nextProcessId);
                if (process) {
                    processQueue.push(process);
                    memoryManager.removeProcessFromStore(nextProcessId);  // Remove the process after enqueueing
                }
                if (!process) {
                    std::cout << "Failed to read process with ID: " << nextProcessId << "\n";
                }

            }
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

    int& idleTicks = coreIdleTicks[coreId];    // Reference to the core's idle tick counter
    int& activeTicks = coreActiveTicks[coreId]; // Reference to the core's active tick counter

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

                int requiredBlocks = nextProcess->mem_allocated / mem_per_frame;
                if (requiredBlocks < 1) {
                    requiredBlocks = 1; // Allocate at least 1 block
                }

                // Check if memory blocks are available
                if (nextProcess->in_mem || memoryManager.allocateBlocks(requiredBlocks, nextProcess->id)) {
                    if (!nextProcess->in_mem) {
                        nextProcess->in_mem = true;
                        nextProcess->mem_allocated = requiredBlocks * mem_per_frame;
                    }

                    process = nextProcess;
                    processQueue.pop();
                    coreAvailable[coreId] = false;
                    coreAssignments[coreId] = process;

                    // Assign random instructions if not already set
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
                    // Not enough memory; send the process back to the queue tail
                    processQueue.push(nextProcess);
                    processQueue.pop();
                }
            }
        }

        if (process) {
            // Execute the process
            int instructions_to_execute = (algorithm == RR) ? std::min(quant_cycles, process->total_ins - process->current_ins) : process->total_ins;

            for (int i = 0; i < instructions_to_execute; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_per_exec+1 * 10));
                process->current_ins++;
                {
                    std::lock_guard<std::mutex> lock(statsMutex);
                    coreActiveTicks[coreId]++; // Increment active ticks
                }

                if (process->dummy) {
                    consoleManager->updateProcessStatus("P" + std::to_string(process->id), "Running", process->current_ins, true);
                }
                else {
                    consoleManager->updateProcessStatus(process->name, "Running", process->current_ins, true);
                }
            }

            // Post-execution handling
            {
                std::lock_guard<std::mutex> lock(queueMutex);

                if (process->current_ins >= process->total_ins) {
                    if (process->dummy) {
                        consoleManager->updateProcessStatus("P" + std::to_string(process->id), "Finished", process->total_ins, false);
                    }
                    else {
                        consoleManager->updateProcessStatus(process->name, "Finished", process->total_ins, false);
                    }

                    memoryManager.releaseBlocks(process->id);
                    delete process;
                    coreAvailable[coreId] = true;
                    coreAssignments.erase(coreId);
                }
                else if (memoryManager.getUsedBlocks() >= memoryManager.getTotalBlocks() && !processQueue.empty()) {
                    if (process->dummy) {
                        consoleManager->updateProcessStatus("P" + std::to_string(process->id), "Waiting", process->current_ins, false);
                        memoryManager.writeProcessToStore(process);
                    }
                    else {
                        consoleManager->updateProcessStatus(process->name, "Waiting", process->current_ins, false);
                        processQueue.push(process);
                        coreAvailable[coreId] = true;
                        coreAssignments.erase(coreId);
                        cv.notify_all();
                    }

                    memoryManager.releaseBlocks(process->id);
                    coreAvailable[coreId] = true;
                    coreAssignments.erase(coreId);
                }
                else if (algorithm == RR) {
                    if (process->dummy) {
                        consoleManager->updateProcessStatus("P" + std::to_string(process->id), "Waiting", process->current_ins, true);
                    }
                    else {
                        consoleManager->updateProcessStatus(process->name, "Waiting", process->current_ins, true);
                    }

                    processQueue.push(process);
                    coreAvailable[coreId] = true;
                    coreAssignments.erase(coreId);
                    cv.notify_all();
                }
            }
        }
        else {
            // Increment idle ticks if no process was executed
            {
                std::lock_guard<std::mutex> lock(statsMutex);
                coreIdleTicks[coreId]++; // Increment idle ticks
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate idle wait
        }
    }
}



void FCFS_Scheduler::addToQueue(string name) {
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(min_mem_per_proc, max_mem_per_proc);
        int randomized_mem = dist(gen);

        std::lock_guard<std::mutex> lock(queueMutex);
        processQueue.push(new Process(name, randomized_mem));
    }
    cv.notify_all();
}

void FCFS_Scheduler::vmstat() {
    // Fetch memory stats
    int totalMemory = memoryManager.getTotalBlocks() * mem_per_frame;
    int usedMemory = memoryManager.getUsedBlocks() * mem_per_frame;
    int freeMemory = totalMemory - usedMemory;

    // Fetch CPU stats
    std::lock_guard<std::mutex> lock(statsMutex); // Ensure thread safety

    int idleCpuTicks = 0;
    int activeCpuTicks = 0;

    for (int i = 0; i < coreCount; ++i) {
        idleCpuTicks += coreIdleTicks[i];
        activeCpuTicks += coreActiveTicks[i];
    }

    int totalTicks = idleCpuTicks + activeCpuTicks;

    // Fetch paging stats
    int numPagedIn = memoryManager.getPageIns();
    int numPagedOut = memoryManager.getPageOuts();

    // Print VM statistics
    system("cls");
    header(); // Custom header function if defined
    std::cout << "---------------------------------------------------------\n";
    std::cout << "vmstat 1.0\n";
    std::cout << "---------------------------------------------------------\n";
    std::cout << "Total memory:\t\t" << totalMemory << " KB\n";
    std::cout << "Used memory:\t\t" << usedMemory << " KB\n";
    std::cout << "Free memory:\t\t" << freeMemory << " KB\n\n";

    std::cout << "CPU Stats:\n";
    std::cout << "Idle CPU ticks:\t\t" << idleCpuTicks << "\n";
    std::cout << "Active CPU ticks:\t" << activeCpuTicks << "\n";
    std::cout << "Total CPU ticks:\t" << totalTicks << "\n\n";

    std::cout << "Paging Stats:\n";
    std::cout << "Pages paged in:\t\t" << numPagedIn << "\n";
    std::cout << "Pages paged out:\t" << numPagedOut << "\n";
    std::cout << "---------------------------------------------------------\n";
}

