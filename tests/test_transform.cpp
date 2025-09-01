#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "transform.h"
#include "udp_receiver.h"
#include "lidar_assembler.h"
#include "udp_packet_structures.h"

void testBasicTransform() {
    std::cout << "=== Basic Transform Test ===" << std::endl;
    
    // Test 1: Identity transform
    glm::vec3 pos(0, 0, 0);
    glm::vec3 rot(0, 0, 0);
    glm::mat4 identity = Transform::createTransform(pos, rot);
    glm::vec3 point(1, 2, 3);
    glm::vec3 transformed = Transform::transformPoint(identity, point);
    std::cout << "Identity transform: (" << point.x << ", " << point.y << ", " << point.z 
              << ") -> (" << transformed.x << ", " << transformed.y << ", " << transformed.z << ")" << std::endl;
    
    // Test 2: Translation only
    pos = glm::vec3(10, 20, 30);
    rot = glm::vec3(0, 0, 0);
    glm::mat4 translation = Transform::createTransform(pos, rot);
    transformed = Transform::transformPoint(translation, point);
    std::cout << "Translation only: (" << point.x << ", " << point.y << ", " << point.z 
              << ") -> (" << transformed.x << ", " << transformed.y << ", " << transformed.z << ")" << std::endl;
    
    // Test 3: 90-degree rotation around Y axis
    pos = glm::vec3(0, 0, 0);
    rot = glm::vec3(0, 90, 0);  // 90 degrees around Y
    glm::mat4 rotation = Transform::createTransform(pos, rot);
    transformed = Transform::transformPoint(rotation, glm::vec3(1, 0, 0));
    std::cout << "90° Y rotation: (1, 0, 0) -> (" 
              << transformed.x << ", " << transformed.y << ", " << transformed.z << ")" << std::endl;
    
    // Test 4: Combined transform
    pos = glm::vec3(5, 0, 5);
    rot = glm::vec3(0, 45, 0);  // 45 degrees around Y
    glm::mat4 combined = Transform::createTransform(pos, rot);
    Transform::printMatrix(combined, "Combined Transform (pos=[5,0,5], rot=[0,45,0])");
    
    std::cout << std::endl;
}

void testWithRoverData() {
    std::cout << "=== Rover Data Transform Test ===" << std::endl;
    std::cout << "Make sure rover emulator is running:" << std::endl;
    std::cout << "  ./rover_emulator 1 --no-noise" << std::endl;
    std::cout << "=====================================\n" << std::endl;
    
    // Create receivers
    UDPReceiver poseReceiver(9001);
    UDPReceiver lidarReceiver(10001);
    
    if (!poseReceiver.isValid() || !lidarReceiver.isValid()) {
        std::cerr << "Failed to create UDP receivers" << std::endl;
        return;
    }
    
    poseReceiver.setNonBlocking(true);
    lidarReceiver.setNonBlocking(true);
    
    LidarAssembler assembler;
    
    // Buffers
    uint8_t poseBuffer[1024];
    uint8_t lidarBuffer[2048];
    
    // Current pose
    PosePacket currentPose;
    bool havePose = false;
    
    std::cout << "Waiting for data..." << std::endl;
    
    auto startTime = std::chrono::steady_clock::now();
    int transformedScans = 0;
    
    while (transformedScans < 5) {  // Process 5 complete scans
        // Check for pose update
        ssize_t poseBytes = poseReceiver.receive(poseBuffer, sizeof(poseBuffer));
        if (poseBytes == sizeof(PosePacket)) {
            memcpy(&currentPose, poseBuffer, sizeof(PosePacket));
            havePose = true;
        }
        
        // Check for LiDAR data
        ssize_t lidarBytes = lidarReceiver.receive(lidarBuffer, sizeof(lidarBuffer));
        if (lidarBytes == sizeof(LidarPacket)) {
            LidarPacket packet;
            memcpy(&packet, lidarBuffer, sizeof(LidarPacket));
            
            if (assembler.addPacket(packet)) {
                // Complete scan available
                LidarAssembler::CompleteScan scan;
                if (assembler.getCompleteScan(scan) && havePose) {
                    transformedScans++;
                    
                    // Create transform from current pose
                    glm::mat4 transform = Transform::poseToMatrix(currentPose);
                    
                    std::cout << std::fixed << std::setprecision(3);
                    std::cout << "\n=== Scan " << transformedScans << " ===" << std::endl;
                    std::cout << "Pose: pos=(" << currentPose.posX << ", " 
                              << currentPose.posY << ", " << currentPose.posZ 
                              << ") rot=(" << currentPose.rotXdeg << "°, " 
                              << currentPose.rotYdeg << "°, " << currentPose.rotZdeg << "°)" << std::endl;
                    
                    // Transform sample points
                    std::cout << "Sample transformed points (first 5):" << std::endl;
                    for (size_t i = 0; i < 5 && i < scan.points.size(); i++) {
                        glm::vec3 local(scan.points[i].x, scan.points[i].y, scan.points[i].z);
                        glm::vec3 world = Transform::transformPoint(transform, local);
                        
                        std::cout << "  Point " << i << ": local(" 
                                  << local.x << ", " << local.y << ", " << local.z 
                                  << ") -> world(" 
                                  << world.x << ", " << world.y << ", " << world.z << ")" << std::endl;
                    }
                    
                    // Transform all points and compute bounds
                    std::vector<glm::vec3> worldPoints = Transform::transformLidarPoints(transform, scan.points);
                    
                    // Find bounding box of transformed points
                    glm::vec3 minBounds(1e9f);
                    glm::vec3 maxBounds(-1e9f);
                    for (const auto& p : worldPoints) {
                        minBounds = glm::min(minBounds, p);
                        maxBounds = glm::max(maxBounds, p);
                    }
                    
                    std::cout << "World bounds: min(" 
                              << minBounds.x << ", " << minBounds.y << ", " << minBounds.z 
                              << ") max(" 
                              << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << ")" << std::endl;
                    
                    glm::vec3 size = maxBounds - minBounds;
                    std::cout << "Bounding box size: (" 
                              << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
                }
            }
        }
        
        // Check timeout
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        if (elapsed > 30) {
            std::cout << "Timeout waiting for data" << std::endl;
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    std::cout << "\n=== Transform Test Complete ===" << std::endl;
    std::cout << "Processed " << transformedScans << " complete scans" << std::endl;
}

int main() {
    std::cout << "=== Coordinate Transform Test ===" << std::endl;
    std::cout << "=================================\n" << std::endl;
    
    // Run basic tests
    testBasicTransform();
    
    // Run tests with real rover data
    testWithRoverData();
    
    return 0;
}
