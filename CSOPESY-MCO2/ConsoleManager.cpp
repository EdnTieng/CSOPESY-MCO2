#include "ConsoleManager.h"
#include "Config.h"
#include "header.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <fstream>  
#include <set>


using namespace std;

// Adds a new process to the ConsoleManager with its initial status, core ID, timestamp, and progress
void ConsoleManager::addProcess(const std::string& name, const std::string& status, int coreId, const std::string& timestamp, int progress, int num_ins, int mem_allocated) {
    for (auto& process : processes) {
        if (process.name == name) {
            // If the process already exists, just update its fields
            process.status = status;
            process.coreId = coreId;
            process.timestamp = timestamp;
            process.progress = progress;
            process.num_ins = num_ins;
            process.mem_allocated = mem_allocated;
            return; // Exit once the process is found and updated
        }
    }
    // If it doesn't exist, create a new one
    processes.push_back({ name, status, coreId, timestamp, progress, num_ins, mem_allocated});
}


// Displays details of a specific process
void ConsoleManager::displayProcess(const std::string& process_name) const {
    for (const auto& process : processes) {
        if (process.name == process_name && process.status != "Finished") {
            system("cls");
            header();
            cout << "Process Name: " << process.name << "\n";
            cout << "Status: " << process.status << "\n";
            cout << "Core ID: " << process.coreId << "\n";
            cout << "Timestamp: " << process.timestamp << "\n";
            cout << "Progress: " << process.progress << "/" << process.num_ins << "\n";
            cout << "Memory Allocated: " << process.mem_allocated << "\n";

            while (true)
            {
                cout << "command: ";
                string user_input;
                getline(cin, user_input);
                if (user_input == "process-smi")
                {
                    cout << "Process Name: " << process.name << "\n";
                    cout << "Status: " << process.status << "\n";
                    cout << "Core ID: " << process.coreId << "\n";
                    cout << "Timestamp: " << process.timestamp << "\n";
                    cout << "Progress: " << process.progress << "/" << process.num_ins << "\n";
                }
                else if (user_input == "exit")
                {
                    system("cls");
                    header();
                    break;
                }
                else
                {
                    cout << "Invalid process comamand";
                }
            }

            return;
        }
    }
    cout << "Process \"" << process_name << "\" not found.\n";
}

// Removes a process from the list based on its name
void ConsoleManager::removeProcess(const std::string& process_name) {
    processes.erase(remove_if(processes.begin(), processes.end(),
        [&process_name](const ProcessInfo& process) {
            return process.name == process_name;
        }), processes.end());
}

// Updates the status and progress of a process
void ConsoleManager::updateProcessStatus(const std::string& process_name, const std::string& status, int progress) {
    for (auto& process : processes) {
        if (process.name == process_name) {
            process.status = status;
            process.progress = progress;
            return;
        }
    }
}

// Lists all processes, displaying both "Running" and "Finished" processes separately
void ConsoleManager::listProcesses() const {
    system("cls");
    // Count unique cores in use for running processes
    set<int> coresInUse;
    for (const auto& process : processes) {
        if (process.status == "Running") {
            coresInUse.insert(process.coreId);
        }
    }

    int totalCores = num_cpu;  // Assuming num_cpu is the total number of cores
    int coresInUseCount = coresInUse.size();
    double coreUsagePercentage = (static_cast<double>(coresInUseCount) / totalCores) * 100;

    cout << "CPU Utilization (" << coreUsagePercentage << "%)\n";
    cout << "Total Cores: " << totalCores << "\n";
    cout << "Cores Available: " << totalCores - coresInUseCount << "\n";


    cout << "------------------------------------\n";

    // Display running processes
    cout << "Running processes:\n";
    for (const auto& process : processes) {
        if (process.status == "Running") {
            cout << process.name << "\t(" << process.timestamp << ")\tCore:\t" << process.coreId
                << "\tProgress: " << process.progress << "/" << process.num_ins << "\n";
        }
    }

    cout << "\nFinished processes:\n";
    for (const auto& process : processes) {
        if (process.status == "Finished") {
            cout << process.name << "\t(" << process.timestamp << ")\tFinished \tProgress: " << process.progress << "/" << process.num_ins << "\n";
        }
    }

    cout << "------------------------------------\n";
}

void ConsoleManager::printProcesses() const {
    std::ofstream outFile("csopesy-log.txt");  // Open the file in write mode

    if (!outFile) {
        cerr << "Error: Could not open csopesy-log.txt for writing.\n";
        return;
    }

    // Calculate CPU utilization
    set<int> coresInUse;
    for (const auto& process : processes) {
        if (process.status == "Running") {
            coresInUse.insert(process.coreId);
        }
    }

    int totalCores = num_cpu;  // Assuming num_cpu is the total number of cores
    int coresInUseCount = coresInUse.size();
    double coreUsagePercentage = (static_cast<double>(coresInUseCount) / totalCores) * 100;

    // Write CPU utilization and core information to the file
    outFile << "------------------------------------\n";
    outFile << "CPU Utilization (" << coreUsagePercentage << "%)\n";
    outFile << "Total Cores: " << totalCores << "\n";
    outFile << "Cores Available: " << totalCores - coresInUseCount << "\n";
    outFile << "------------------------------------\n";

    // Write running processes to the file
    outFile << "Running processes:\n";
    for (const auto& process : processes) {
        if (process.status == "Running") {
            outFile << process.name << "\t(" << process.timestamp << ")\tCore:\t" << process.coreId
                << "\tProgress: " << process.progress << "/" << process.num_ins << "\n";
        }
    }

    // Write finished processes to the file
    outFile << "\nFinished processes:\n";
    for (const auto& process : processes) {
        if (process.status == "Finished") {
            outFile << process.name << "\t(" << process.timestamp << ")\tFinished \tProgress: " << process.progress << "/" << process.num_ins << "\n";
        }
    }

    outFile << "------------------------------------\n";
    outFile.close();  // Close the file

    cout << "Process log has been written to csopesy-log.txt.\n";
}

void ConsoleManager::printMemoryStamp(int qq, int maxOverallMem, int currentMemoryUsage) const {
    // Create a file name with the "memory_stamp_<qq>.txt" format
    std::ostringstream fileName;
    fileName << "memory_stamp_" << qq << ".txt";
    std::ofstream outFile(fileName.str());  // Open the file in write mode

    if (!outFile) {
        std::cerr << "Error: Could not open " << fileName.str() << " for writing.\n";
        return;
    }

    // Generate a timestamp
    auto now = std::time(nullptr);
    struct tm local_time;
    localtime_s(&local_time, &now);
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%m/%d/%Y %I:%M:%S %p");
    std::string timestamp = oss.str();

    // Count the number of processes in memory (those with "Running" status)
    int processesInMemory = 0;
    for (const auto& process : processes) {
        if (process.status == "Running") {
            ++processesInMemory;
        }
    }
    // Calculate available memory
    int availableMemory = maxOverallMem - currentMemoryUsage;
    int top_mem = maxOverallMem;
    int bot_mem = 0;
    // Write the information to the file
    outFile << "Timestamp: " << timestamp << "\n";
    outFile << "Processes in Memory: " << processesInMemory << "\n";
    outFile << "Available Memory: " << availableMemory << "\n";

    outFile << "----end---- = " << maxOverallMem << "\n";
    for (const auto& process : processes) {
        if (process.status == "Running") {
            outFile << top_mem <<"\n";
            outFile << process.name <<"\n";
            bot_mem = top_mem - process.mem_allocated;
            outFile << bot_mem <<"\n";
            top_mem = bot_mem - process.mem_allocated;
        }
    }
    outFile << "----start---- = 0\n";
    outFile.close();  // Close the file
}
