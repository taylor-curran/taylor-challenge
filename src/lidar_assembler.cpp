#include "lidar_assembler.h"
#include <iostream>
#include <algorithm>

LidarAssembler::LidarAssembler() 
    : totalChunksReceived_(0), totalScansCompleted_(0) {
}

bool LidarAssembler::addPacket(const LidarPacket& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    totalChunksReceived_++;
    
    double timestamp = packet.header.timestamp;
    uint32_t chunkIndex = packet.header.chunkIndex;
    uint32_t totalChunks = packet.header.totalChunks;
    uint32_t pointsInChunk = packet.header.pointsInThisChunk;
    
    // Find or create partial scan for this timestamp
    auto& partial = partialScans_[timestamp];
    partial.timestamp = timestamp;
    partial.totalChunks = totalChunks;
    partial.lastUpdateTime = std::chrono::steady_clock::now();
    
    // Add points from this chunk
    std::vector<LidarPoint> points;
    points.reserve(pointsInChunk);
    for (uint32_t i = 0; i < pointsInChunk && i < MAX_LIDAR_POINTS_PER_PACKET; ++i) {
        points.push_back(packet.points[i]);
    }
    
    // Store chunk (will overwrite if duplicate received)
    partial.chunks[chunkIndex] = std::move(points);
    
    // Check if scan is complete
    if (partial.chunks.size() == totalChunks) {
        // Assemble complete scan
        CompleteScan complete;
        complete.timestamp = timestamp;
        complete.totalChunks = totalChunks;
        
        // Combine all chunks in order
        for (const auto& [idx, chunkPoints] : partial.chunks) {
            complete.points.insert(complete.points.end(), 
                                 chunkPoints.begin(), 
                                 chunkPoints.end());
        }
        
        // Move to complete scans
        completeScans_.push_back(std::move(complete));
        
        // Remove from partial scans
        partialScans_.erase(timestamp);
        
        totalScansCompleted_++;
        
        return true;  // Scan completed
    }
    
    return false;  // Scan not yet complete
}

bool LidarAssembler::hasCompleteScan() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return !completeScans_.empty();
}

bool LidarAssembler::getCompleteScan(CompleteScan& scan) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (completeScans_.empty()) {
        return false;
    }
    
    // Get oldest scan (front of vector)
    scan = std::move(completeScans_.front());
    completeScans_.erase(completeScans_.begin());
    
    return true;
}

void LidarAssembler::cleanupStaleScans(double maxAgeSeconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto maxAge = std::chrono::duration<double>(maxAgeSeconds);
    
    // Find and remove stale partial scans
    auto it = partialScans_.begin();
    while (it != partialScans_.end()) {
        auto age = now - it->second.lastUpdateTime;
        if (age > maxAge) {
            std::cout << "Cleaning up stale partial scan at timestamp " 
                      << it->first 
                      << " (had " << it->second.chunks.size() 
                      << "/" << it->second.totalChunks << " chunks)" 
                      << std::endl;
            it = partialScans_.erase(it);
        } else {
            ++it;
        }
    }
}

size_t LidarAssembler::getPartialScanCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return partialScans_.size();
}

size_t LidarAssembler::getCompleteScanCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return completeScans_.size();
}
