#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Data structures matching the emulator (from README.md)
#pragma pack(push, 1)
struct PosePacket {
    double timestamp;    // Seconds since emulator start
    float  posX;         // X position (units)
    float  posY;         // Y position (units)
    float  posZ;         // Z position (units)
    float  rotXdeg;      // Roll or X rotation (degrees)
    float  rotYdeg;      // Pitch or Y rotation (degrees)
    float  rotZdeg;      // Yaw or Z rotation (degrees)
};

struct LidarPacketHeader {
    double   timestamp;       // Same as PosePacket timestamp
    uint32_t chunkIndex;      // Which chunk this is
    uint32_t totalChunks;     // Total chunks for this scan
    uint32_t pointsInThisChunk;
};

struct LidarPoint {
    float x;
    float y;
    float z;
};

static const size_t MAX_LIDAR_POINTS_PER_PACKET = 100;

struct LidarPacket {
    LidarPacketHeader header;
    LidarPoint points[MAX_LIDAR_POINTS_PER_PACKET];
};
#pragma pack(pop)

void listen_pose(int rover_id) {
    int port = 9000 + rover_id;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "POSE: Failed to create socket" << std::endl;
        return;
    }
    
    // Socket quality-of-life improvements
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    int buf = 4 * 1024 * 1024; // 4MB buffer
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buf, sizeof(buf));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "POSE: Failed to bind to port " << port << std::endl;
        close(sockfd);
        return;
    }

    // Set socket timeout
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    std::cout << "Listening for pose data on port " << port << "..." << std::endl;

    for (int i = 0; i < 5; i++) {
        PosePacket packet;
        ssize_t bytes_received = recv(sockfd, &packet, sizeof(packet), 0);
        
        if (bytes_received > 0) {
            if (bytes_received == sizeof(PosePacket)) {
                std::cout << "POSE: t=" << std::fixed << std::setprecision(3) << packet.timestamp
                         << ", pos=(" << std::setprecision(2) << packet.posX << "," << packet.posY << "," << packet.posZ << ")"
                         << ", rot=(" << std::setprecision(1) << packet.rotXdeg << "," << packet.rotYdeg << "," << packet.rotZdeg << ")"
                         << std::endl;
            } else {
                std::cout << "POSE: Received " << bytes_received << " bytes (expected " << sizeof(PosePacket) << ")" << std::endl;
            }
        } else {
            std::cout << "POSE: No data received on port " << port << std::endl;
            break;
        }
    }

    close(sockfd);
}

void listen_lidar(int rover_id) {
    int port = 10000 + rover_id;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "LIDAR: Failed to create socket" << std::endl;
        return;
    }
    
    // Socket quality-of-life improvements
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    int buf = 4 * 1024 * 1024; // 4MB buffer
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buf, sizeof(buf));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "LIDAR: Failed to bind to port " << port << std::endl;
        close(sockfd);
        return;
    }

    // Set socket timeout
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    std::cout << "Listening for LiDAR data on port " << port << "..." << std::endl;

    for (int i = 0; i < 3; i++) {
        LidarPacket packet;
        ssize_t bytes_received = recv(sockfd, &packet, sizeof(packet), 0);
        
        if (bytes_received > 0) {
            if (bytes_received >= sizeof(LidarPacketHeader)) {
                std::cout << "LIDAR: t=" << std::fixed << std::setprecision(3) << packet.header.timestamp
                         << ", chunk " << (packet.header.chunkIndex + 1) << "/" << packet.header.totalChunks
                         << ", " << packet.header.pointsInThisChunk << " points" << std::endl;
            } else {
                std::cout << "LIDAR: Received " << bytes_received << " bytes (expected >=" << sizeof(LidarPacketHeader) << ")" << std::endl;
            }
        } else {
            std::cout << "LIDAR: No data received on port " << port << std::endl;
            break;
        }
    }

    close(sockfd);
}

int main() {
    int rover_id = 1;
    std::cout << "Testing rover " << rover_id << " emulator..." << std::endl;

    // Start listeners in separate threads
    std::thread pose_thread(listen_pose, rover_id);
    std::thread lidar_thread(listen_lidar, rover_id);

    // Wait for threads to complete
    pose_thread.join();
    lidar_thread.join();

    std::cout << "UDP listener test complete." << std::endl;
    return 0;
}
