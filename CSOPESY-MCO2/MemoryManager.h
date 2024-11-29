#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <fstream>
#include <sstream>
#include "Process.h"

using namespace std;
class MemoryManager {
private:
    int totalBlocks;                   // Total number of memory blocks
    int blocksUsed;                    // Number of blocks currently in use
    std::vector<bool> memoryBlocks;    // Block availability (true = used, false = free)
    int memPerFrame;                   // Memory per frame

    std::string backingStoreFilePath = "backing_store.txt"; // Path to the backing store file

public:
    // Constructor
    MemoryManager(int maxOverallMem, int memPerFrame);

    // Allocate a specified number of blocks
    bool allocateBlocks(int requiredBlocks);

    // Release a specified number of blocks
    void releaseBlocks(int requiredBlocks);

    // Get current memory usage
    int getUsedBlocks() const;

    // Get total memory blocks
    int getTotalBlocks() const;

    // Write a process to the backing store
    void writeProcessToStore(const Process* process);

    // Read a process from the backing store by ID
    Process* readProcessFromStore(int processName);

    // Remove a process from the backing store by ID
    void removeProcessFromStore(int processId);

    // Check if the backing store is empty
    bool isBackingStoreEmpty() const;

    int peekNextProcessIdFromStore();
};

#endif // MEMORY_MANAGER_H
