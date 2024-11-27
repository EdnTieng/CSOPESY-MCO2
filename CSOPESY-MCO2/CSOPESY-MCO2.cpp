#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <iomanip> // For put_time
#include <sstream> // For ostringstream
#include <random>  // For random number generation
#include "header.h"
#include "ConsoleManager.h"
#include "FCFS.h"
#include "Config.h"

using namespace std;
vector<ProcessInfo> processes;

int main() {
    ConsoleManager consoleManager;
    header();
    bool running = true;
    bool initialized = false;

    // Random number generator setup
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist;

    // Scheduler pointer
    FCFS_Scheduler* scheduler = nullptr;

    while (running) {
        cout << "command: ";
        string user_input;
        getline(cin, user_input);

        if (user_input == "initialize") {
            cout << "executing initialize...\n";

            // Load config file
            ifstream configFile("config.txt");
            if (configFile.is_open()) {
                string key;
                while (configFile >> key) {
                    if (key == "num_cpu") {
                        configFile >> num_cpu;
                    }
                    else if (key == "scheduler") {
                        configFile >> sched;
                    }
                    else if (key == "quantum_cycles") {
                        configFile >> quant_cycles;
                    }
                    else if (key == "batch_process_freq") {
                        configFile >> batch_process_freq;
                    }
                    else if (key == "min_ins") {
                        configFile >> min_ins;
                    }
                    else if (key == "max_ins") {
                        configFile >> max_ins;
                    }
                    else if (key == "delay_per_exec") {
                        configFile >> delay_per_exec;
                    }
                    else if (key == "max_overall_mem") {
                        configFile >> max_overall_mem;
                    }
                    else if (key == "mem_per_frame") {
                        configFile >> mem_per_frame;
                    }
                    else if (key == "min_mem_per_proc") {
                        configFile >> min_mem_per_proc;
                    }
                    else if (key == "max_mem_per_proc") {
                        configFile >> max_mem_per_proc;
                    }
                    else {
                        cout << "Unknown configuration key: " << key << "\n";
                    }
                }
                configFile.close();
                assignConfig(num_cpu, sched, quant_cycles, batch_process_freq, min_ins, max_ins, delay_per_exec);
                assignConfig2(max_overall_mem, mem_per_frame, min_mem_per_proc, max_mem_per_proc);
                initialized = true;                                                                              
                
                // Initialize FCFS_Scheduler with config values                                                  
                scheduler = new FCFS_Scheduler(num_cpu, &consoleManager);
                scheduler->start();

                // Set the distribution range
                dist = uniform_int_distribution<>(min_ins, max_ins);
            }
            else {
                cout << "Unable to open config.txt\n";
            }

            while (initialized) {
                cout << "command: ";
                getline(cin, user_input);

                if (user_input.substr(0, 6) == "screen") {
                    string command = user_input.substr(7);
                    stringstream ss(command);
                    string action, name;
                    ss >> action >> name;

                    if (action == "-s") {
                        auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
                        struct tm local_time;
                        localtime_s(&local_time, &now);
                        ostringstream oss;
                        oss << put_time(&local_time, "%m/%d/%Y, %I:%M:%S %p");
                        string timestamp = oss.str();

                        int random_instructions = dist(gen);
                        scheduler->addToQueue(name);
                    }
                    else if (action == "-r") {
                        consoleManager.displayProcess(name);
                    }
                    else if (action == "-ls") {
                        consoleManager.listProcesses();
                    }
                    else {
                        cout << "Invalid screen option\n";
                    }
                }
                else if (user_input == "scheduler-test") {
                    cout << "Starting scheduler test...\n";
                    scheduler->schedulingTestStart(true);
                }
                else if (user_input == "scheduler-stop") {
                    cout << "Stopping scheduler test...\n";
                    scheduler->schedulingTestStart(false);
                }
                else if (user_input == "report-util") {
                    cout << "executing report-util\n";
                    consoleManager.printProcesses();
                }
                else if (user_input == "clear" || user_input == "cls") {
                    system("cls");
                    header();
                }
                else if (user_input == "process-smi")
                {
                    consoleManager.process_smi();
                }
                else if (user_input == "vmstat")
                {
                    consoleManager.vmstat();
                }
                else if (user_input == "exit") {
                    initialized = false;

                    scheduler->stop();
                    cout << "exiting\n";
                    delete scheduler; // Clean up the dynamically allocated scheduler
                    scheduler = nullptr;
                    system("cls");
                    header();
                }
                else {
                    cout << "Invalid Command\n";
                }
            }
        }
        else {
            cout << "Invalid Command\n";
        }
    }
}
