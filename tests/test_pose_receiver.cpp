#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cstring>
#include "udp_receiver.h"
#include "udp_packet_structures.h"

void printPosePacket(const PosePacket& pose) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Timestamp: " << std::setw(8) << pose.timestamp << "s | ";
    std::cout << "Pos: (" 
              << std::setw(7) << pose.posX << ", " 
              << std::setw(7) << pose.posY << ", " 
              << std::setw(7) << pose.posZ << ") | ";
    std::cout << "Rot: (" 
              << std::setw(7) << pose.rotXdeg << "°, " 
              << std::setw(7) << pose.rotYdeg << "°, " 
              << std::setw(7) << pose.rotZdeg << "°)" << std::endl;
}

int main() {
    std::cout << "=== UDP Pose Packet Receiver Test ===" << std::endl;
    std::cout << "Make sure rover emulator is running:" << std::endl;
    std::cout << "  ./rover_emulator 1 --no-noise" << std::endl;
    std::cout << "=====================================\n" << std::endl;
    
    // Create UDP receiver for rover 1 pose data (port 9001)
    UDPReceiver poseReceiver(9001);
    
    if (!poseReceiver.isValid()) {
        std::cerr << "Failed to create UDP receiver on port 9001" << std::endl;
        return 1;
    }
    
    // Set to non-blocking mode
    poseReceiver.setNonBlocking(true);
    
    // Buffer for receiving data
    uint8_t buffer[1024];
    
    // Statistics
    int packetsReceived = 0;
    auto startTime = std::chrono::steady_clock::now();
    auto lastReceiveTime = startTime;
    
    std::cout << "Listening for pose packets on port 9001..." << std::endl;
    std::cout << "Press Ctrl+C to stop\n" << std::endl;
    
    while (true) {
        ssize_t bytesReceived = poseReceiver.receive(buffer, sizeof(buffer));
        
        if (bytesReceived > 0) {
            // Check if we received a complete PosePacket
            if (bytesReceived == sizeof(PosePacket)) {
                PosePacket pose;
                memcpy(&pose, buffer, sizeof(PosePacket));
                
                packetsReceived++;
                auto now = std::chrono::steady_clock::now();
                
                // Print every 10th packet to avoid flooding console
                if (packetsReceived % 10 == 0) {
                    printPosePacket(pose);
                    
                    // Calculate and display stats
                    auto totalDuration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
                    if (totalDuration > 0) {
                        float avgRate = static_cast<float>(packetsReceived) / totalDuration;
                        std::cout << "  [Stats: " << packetsReceived << " packets, "
                                  << std::setprecision(1) << avgRate << " Hz avg]" << std::endl;
                    }
                }
                
                lastReceiveTime = now;
            } else {
                std::cout << "Warning: Received " << bytesReceived 
                          << " bytes (expected " << sizeof(PosePacket) << ")" << std::endl;
            }
        } else if (bytesReceived == -1) {
            // No data available (non-blocking mode)
            // Check for timeout
            auto now = std::chrono::steady_clock::now();
            auto timeSinceLastPacket = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastReceiveTime).count();
            
            if (packetsReceived > 0 && timeSinceLastPacket > 1000) {
                std::cout << "\nNo packets received for 1 second - rover may have stopped" << std::endl;
                lastReceiveTime = now;  // Reset to avoid repeated messages
            }
            
            // Small sleep to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
            // Error occurred
            std::cerr << "Error receiving data" << std::endl;
            break;
        }
    }
    
    return 0;
}
