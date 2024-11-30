#include "MemoryManager.h"
#include <iostream>

MemoryManager::MemoryManager(int maxOverallMem, int memPerFrame)
    : blocksUsed(0), memPerFrame(memPerFrame), pageIns(0), pageOuts(0) {
    totalBlocks = maxOverallMem / memPerFrame;
    memoryBlocks.resize(totalBlocks, false); // All blocks are initially free
}

bool MemoryManager::allocateBlocks(int requiredBlocks, int processId) {
    if (isPagingMode()) {
        int allocatedBlocks = 0;
        std::vector<int> allocatedPages;

        // Find and allocate individual frames (non-contiguous allowed)
        for (int i = 0; i < totalBlocks; ++i) {
            if (!memoryBlocks[i]) { // Free frame
                memoryBlocks[i] = true;
                allocatedPages.push_back(i);
                allocatedBlocks++;

                if (allocatedBlocks == requiredBlocks) {
                    processPageMap[processId] = allocatedPages; // Record page allocations
                    blocksUsed += requiredBlocks;
                    return true;
                }
            }
        }

        // Rollback in case of partial allocation
        for (int page : allocatedPages) {
            memoryBlocks[page] = false;
        }
        return false; // Not enough free frames
    }
    else {
        // Flat memory allocation logic
        int contiguousFreeBlocks = 0;
        for (int i = 0; i < totalBlocks; ++i) {
            if (!memoryBlocks[i]) {
                contiguousFreeBlocks++;
                if (contiguousFreeBlocks == requiredBlocks) {
                    for (int j = i - requiredBlocks + 1; j <= i; ++j) {
                        memoryBlocks[j] = true;
                    }
                    blocksUsed += requiredBlocks;
                    return true;
                }
            }
            else {
                contiguousFreeBlocks = 0; // Reset counter
            }
        }
        return false; // Not enough contiguous blocks
    }
}


void MemoryManager::releaseBlocks(int processId) {
    if (isPagingMode()) {
        // Release pages allocated to the process
        auto it = processPageMap.find(processId);
        if (it != processPageMap.end()) {
            for (int pageIndex : it->second) {
                memoryBlocks[pageIndex] = false; // Mark frame as free
            }
            blocksUsed -= it->second.size(); // Update used blocks
            processPageMap.erase(it);        // Remove from the map
        }
    }
    else {
        // Flat memory logic
        int releasedBlocks = 0;
        for (int i = 0; i < totalBlocks && releasedBlocks < blocksUsed; ++i) {
            if (memoryBlocks[i]) {
                memoryBlocks[i] = false;
                releasedBlocks++;
            }
        }
        blocksUsed -= releasedBlocks;
    }
}


int MemoryManager::getUsedBlocks() const {
    return blocksUsed;
}

int MemoryManager::getTotalBlocks() const {
    return totalBlocks;
}

void MemoryManager::writeProcessToStore(const Process* process) {
    std::ofstream outFile(backingStoreFilePath, std::ios::app);
    if (!outFile.is_open()) {
        throw std::runtime_error("Unable to open backing store file.");
    }

    outFile << process->id << ","
        << process->name << ","
        << process->current_ins << ","
        << process->total_ins << ","
        << process->mem_allocated;

    
    outFile << "\n";

    outFile.close();
    pageOuts++;
}


Process* MemoryManager::readProcessFromStore(int processId) {
    std::ifstream inFile(backingStoreFilePath);
    if (!inFile.is_open()) {
        throw std::runtime_error("Unable to open backing store file.");
    }

    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream ss(line);

        int id;
        std::string name;
        int currentIns = 0, totalIns = 0, memAllocated = 0;

        // Parse the line in the correct order
        char delimiter;
        if (ss >> id >> delimiter && delimiter == ',' &&  // Ensure proper ID and delimiter
            std::getline(ss, name, ',') &&                // Read the name until the next delimiter
            ss >> currentIns >> delimiter && delimiter == ',' &&
            ss >> totalIns >> delimiter && delimiter == ',' &&
            ss >> memAllocated) {

            // If the process ID matches, reconstruct the process
            if (id == processId) {
                inFile.close();
                return new Process(id, name, currentIns, totalIns, memAllocated);
            }
        }
    }
    pageIns++;
    cout << pageIns;
    inFile.close();
    return nullptr; // Process not found
}


void MemoryManager::removeProcessFromStore(int processId) {
    std::ifstream inFile(backingStoreFilePath);
    std::ofstream tempFile("temp.txt");

    if (!inFile.is_open() || !tempFile.is_open()) {
        throw std::runtime_error("Unable to open files for modification.");
    }

    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream ss(line);
        int id;
        ss >> id;

        if (id != processId) {
            tempFile << line << "\n"; // Write non-matching lines to temp file
        }
    }

    inFile.close();
    tempFile.close();

    // Replace the original file with the temp file
    std::remove(backingStoreFilePath.c_str());
    std::rename("temp.txt", backingStoreFilePath.c_str());
}

bool MemoryManager::isBackingStoreEmpty() const {
    std::ifstream inFile(backingStoreFilePath);
    return inFile.peek() == std::ifstream::traits_type::eof();
}

int MemoryManager::peekNextProcessIdFromStore() {
    ifstream file("backing_store.txt");
    
    if (file.is_open()) {
        int id;
        string name;
        int memory; // We assume memory size might also be recorded, but you can skip this if not needed
        // Read the first line (ID)
        if (file >> id) {
            return id;  // Return the ID of the first process in the file
        }
    }
    return -1; // Return -1 if no process is found
}

bool MemoryManager::isPagingMode() const {
    return max_overall_mem > mem_per_frame;
}

int MemoryManager::getPageIns() const {
    return pageIns;
}

int MemoryManager::getPageOuts() const {
    return pageOuts;
}
