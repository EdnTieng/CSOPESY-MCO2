#include "MemoryManager.h"
#include <iostream>
MemoryManager::MemoryManager(int maxOverallMem, int memPerFrame)
    : blocksUsed(0), memPerFrame(memPerFrame) {
    totalBlocks = maxOverallMem / memPerFrame;
    memoryBlocks.resize(totalBlocks, false); // All blocks are initially free
}

bool MemoryManager::allocateBlocks(int requiredBlocks) {
    int contiguousFreeBlocks = 0;

    // Find a range of contiguous free blocks
    for (int i = 0; i < totalBlocks; ++i) {
        if (!memoryBlocks[i]) {
            contiguousFreeBlocks++;
            if (contiguousFreeBlocks == requiredBlocks) {
                // Allocate the blocks
                for (int j = i - requiredBlocks + 1; j <= i; ++j) {
                    memoryBlocks[j] = true;
                }
                blocksUsed += requiredBlocks;
                return true; // Allocation successful
            }
        }
        else {
            contiguousFreeBlocks = 0; // Reset counter if a block is occupied
        }
    }
    return false; // Not enough contiguous blocks available
}

void MemoryManager::releaseBlocks(int requiredBlocks) {
    int releasedBlocks = 0;

    // Free up `requiredBlocks` blocks (iterate through the map)
    for (int i = 0; i < totalBlocks && releasedBlocks < requiredBlocks; ++i) {
        if (memoryBlocks[i]) {
            memoryBlocks[i] = false; // Mark block as free
            releasedBlocks++;
        }
    }
    blocksUsed -= releasedBlocks;
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
        << process->mem_allocated << "\n";

    outFile.close();
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
