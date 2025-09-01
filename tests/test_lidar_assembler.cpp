#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cstring>
#include "udp_receiver.h"
#include "lidar_assembler.h"
#include "udp_packet_structures.h"

void printStats(const LidarAssembler& assembler) {
    std::cout << "  [Chunks: " << assembler.getTotalChunksReceived() 
              << " | Complete scans: " << assembler.getTotalScansCompleted()
              << " | Partial: " << assembler.getPartialScanCount()
              << " | Ready: " << assembler.getCompleteScanCount() << "]" << std::endl;
}

int main() {
    std::cout << "=== LiDAR Chunk Assembler Test ===" << std::endl;
    std::cout << "Make sure rover emulator is running:" << std::endl;
    std::cout << "  ./rover_emulator 1 --no-noise" << std::endl;
    std::cout << "===================================\n" << std::endl;
    
    // Create UDP receiver for rover 1 LiDAR data (port 10001)
    UDPReceiver lidarReceiver(10001);
    
    if (!lidarReceiver.isValid()) {
        std::cerr << "Failed to create UDP receiver on port 10001" << std::endl;
        return 1;
    }
    
    // Set to non-blocking mode
    lidarReceiver.setNonBlocking(true);
    
    // Create LiDAR assembler
    LidarAssembler assembler;
    
    // Buffer for receiving data
    uint8_t buffer[2048];  // Large enough for LidarPacket (1220 bytes)
    
    // Statistics
    int packetsReceived = 0;
    int scansCompleted = 0;
    auto startTime = std::chrono::steady_clock::now();
    auto lastReceiveTime = startTime;
    auto lastCleanupTime = startTime;
    
    std::cout << "Listening for LiDAR packets on port 10001..." << std::endl;
    std::cout << "Press Ctrl+C to stop\n" << std::endl;
    
    while (true) {
        ssize_t bytesReceived = lidarReceiver.receive(buffer, sizeof(buffer));
        
        if (bytesReceived > 0) {
            // Check if we received a complete LidarPacket
            if (bytesReceived == sizeof(LidarPacket)) {
                LidarPacket packet;
                memcpy(&packet, buffer, sizeof(LidarPacket));
                
                packetsReceived++;
                
                // Add packet to assembler
                bool scanComplete = assembler.addPacket(packet);
                
                if (scanComplete) {
                    scansCompleted++;
                    
                    // Get the complete scan
                    LidarAssembler::CompleteScan scan;
                    if (assembler.getCompleteScan(scan)) {
                        std::cout << std::fixed << std::setprecision(3);
                        std::cout << "Full scan received: " 
                                  << scan.points.size() << " points"
                                  << " | Timestamp: " << scan.timestamp 
                                  << " | Chunks: " << scan.totalChunks << std::endl;
                        
                        // Print sample of first and last points
                        if (scan.points.size() > 0) {
                            const auto& first = scan.points.front();
                            const auto& last = scan.points.back();
                            std::cout << "  First point: (" 
                                      << first.x << ", " << first.y << ", " << first.z << ")"
                                      << " | Last point: (" 
                                      << last.x << ", " << last.y << ", " << last.z << ")" 
                                      << std::endl;
                        }
                        
                        printStats(assembler);
                    }
                } else {
                    // Print progress every 10 chunks
                    if (packetsReceived % 10 == 0) {
                        std::cout << "Received chunk " << packet.header.chunkIndex 
                                  << "/" << packet.header.totalChunks
                                  << " for timestamp " << packet.header.timestamp 
                                  << std::endl;
                        printStats(assembler);
                    }
                }
                
                lastReceiveTime = std::chrono::steady_clock::now();
            } else {
                std::cout << "Warning: Received " << bytesReceived 
                          << " bytes (expected " << sizeof(LidarPacket) << ")" << std::endl;
            }
        } else if (bytesReceived == -1) {
            // No data available (non-blocking mode)
            auto now = std::chrono::steady_clock::now();
            
            // Check for timeout
            auto timeSinceLastPacket = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastReceiveTime).count();
            if (packetsReceived > 0 && timeSinceLastPacket > 1000) {
                std::cout << "\nNo packets received for 1 second - rover may have stopped" << std::endl;
                lastReceiveTime = now;  // Reset to avoid repeated messages
            }
            
            // Periodically clean up stale partial scans
            auto timeSinceCleanup = std::chrono::duration_cast<std::chrono::seconds>(now - lastCleanupTime).count();
            if (timeSinceCleanup > 5) {
                assembler.cleanupStaleScans(2.0);
                lastCleanupTime = now;
            }
            
            // Small sleep to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
            // Error occurred
            std::cerr << "Error receiving data" << std::endl;
            break;
        }
    }
    
    // Final stats
    std::cout << "\n=== Final Statistics ===" << std::endl;
    std::cout << "Total chunks received: " << assembler.getTotalChunksReceived() << std::endl;
    std::cout << "Total scans completed: " << assembler.getTotalScansCompleted() << std::endl;
    std::cout << "Partial scans remaining: " << assembler.getPartialScanCount() << std::endl;
    
    return 0;
}
