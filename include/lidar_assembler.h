#ifndef LIDAR_ASSEMBLER_H
#define LIDAR_ASSEMBLER_H

#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include "udp_packet_structures.h"

// Assembles LiDAR chunks into complete scans
class LidarAssembler {
public:
    // Container for a complete LiDAR scan
    struct CompleteScan {
        double timestamp;
        std::vector<LidarPoint> points;
        size_t totalChunks;
        
        CompleteScan() : timestamp(0), totalChunks(0) {}
    };
    
    // Container for chunks being assembled
    struct PartialScan {
        double timestamp;
        std::map<uint32_t, std::vector<LidarPoint>> chunks;  // chunkIndex -> points
        uint32_t totalChunks;
        std::chrono::steady_clock::time_point lastUpdateTime;
        
        PartialScan() : timestamp(0), totalChunks(0) {}
    };
    
    LidarAssembler();
    ~LidarAssembler() = default;
    
    // Add a received LiDAR packet to the assembler
    // Returns true if this completes a scan
    bool addPacket(const LidarPacket& packet);
    
    // Check if a complete scan is available
    bool hasCompleteScan() const;
    
    // Get and remove the oldest complete scan
    // Returns false if no complete scan available
    bool getCompleteScan(CompleteScan& scan);
    
    // Clean up old partial scans that haven't been completed
    // (e.g., due to dropped packets)
    void cleanupStaleScans(double maxAgeSeconds = 2.0);
    
    // Get statistics
    size_t getPartialScanCount() const;
    size_t getCompleteScanCount() const;
    size_t getTotalChunksReceived() const { return totalChunksReceived_; }
    size_t getTotalScansCompleted() const { return totalScansCompleted_; }
    
private:
    // Partial scans being assembled (key: timestamp)
    std::map<double, PartialScan> partialScans_;
    
    // Complete scans ready for retrieval
    std::vector<CompleteScan> completeScans_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Statistics
    size_t totalChunksReceived_;
    size_t totalScansCompleted_;
};

#endif // LIDAR_ASSEMBLER_H
