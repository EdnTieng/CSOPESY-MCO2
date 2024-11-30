#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <fstream>
#include <sstream>
#include "Process.h"
#include "Config.h"
#include <unordered_map>

using namespace std;
class MemoryManager {
private:
    int totalBlocks;                   // Total number of memory blocks
    int blocksUsed;                    // Number of blocks currently in use
    std::vector<bool> memoryBlocks;    // Block availability (true = used, false = free)
    int memPerFrame;                   // Memory per frame

    int pageIns;  // Tracks the number of page-ins
    int pageOuts; // Tracks the number of page-outs

    std::string backingStoreFilePath = "backing_store.txt"; // Path to the backing store file
    std::unordered_map<int, std::vector<int>> processPageMap; // Process ID -> Vector of page indices

public:
    // Constructor
    MemoryManager(int maxOverallMem, int memPerFrame);

    // Allocate a specified number of blocks
    bool allocateBlocks(int requiredBlocks, int processId);

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

    bool isPagingMode() const;

    int getPageIns() const;  // Getter for page-ins

    int getPageOuts() const; // Getter for page-outs
};

#endif // MEMORY_MANAGER_H
