#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <sstream>

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

// Structures to store collected data
struct CollectedPoseData {
    std::vector<uint8_t> raw_bytes;
    PosePacket packet;
    size_t size;
};

struct CollectedLidarData {
    std::vector<uint8_t> raw_bytes;
    LidarPacket packet;
    size_t size;
};

void print_raw_values_pose(const PosePacket* packet) {
    std::cout << "  Raw values: [" 
              << packet->timestamp << ", "
              << packet->posX << ", "
              << packet->posY << ", " 
              << packet->posZ << ", "
              << packet->rotXdeg << ", "
              << packet->rotYdeg << ", "
              << packet->rotZdeg << "]" << std::endl;
}

void print_raw_values_lidar(const LidarPacketHeader* header) {
    std::cout << "  Raw header values: ["
              << header->timestamp << ", "
              << header->chunkIndex << ", "
              << header->totalChunks << ", "
              << header->pointsInThisChunk << "]" << std::endl;
}

void collect_pose(int rover_id, std::vector<CollectedPoseData>& pose_data) {
    int port = 9000 + rover_id;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return;
    }
    
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    int buf = 4 * 1024 * 1024;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buf, sizeof(buf));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return;
    }

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    for (int i = 0; i < 2; i++) {
        CollectedPoseData data;
        ssize_t bytes_received = recv(sockfd, &data.packet, sizeof(data.packet), 0);
        
        if (bytes_received > 0 && bytes_received == sizeof(PosePacket)) {
            data.size = bytes_received;
            data.raw_bytes.resize(bytes_received);
            memcpy(data.raw_bytes.data(), &data.packet, bytes_received);
            pose_data.push_back(data);
        }
    }

    close(sockfd);
}

void collect_lidar(int rover_id, std::vector<CollectedLidarData>& lidar_data) {
    int port = 10000 + rover_id;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return;
    }
    
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    int buf = 4 * 1024 * 1024;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buf, sizeof(buf));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return;
    }

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    for (int i = 0; i < 2; i++) {
        CollectedLidarData data;
        ssize_t bytes_received = recv(sockfd, &data.packet, sizeof(data.packet), 0);
        
        if (bytes_received >= sizeof(LidarPacketHeader)) {
            data.size = bytes_received;
            data.raw_bytes.resize(bytes_received);
            memcpy(data.raw_bytes.data(), &data.packet, bytes_received);
            lidar_data.push_back(data);
        }
    }

    close(sockfd);
}

int main() {
    int rover_id = 1;
    std::cout << "Collecting data from rover " << rover_id << " emulator..." << std::endl;

    std::vector<CollectedPoseData> pose_data;
    std::vector<CollectedLidarData> lidar_data;

    // Start collectors in separate threads
    std::thread pose_thread(collect_pose, rover_id, std::ref(pose_data));
    std::thread lidar_thread(collect_lidar, rover_id, std::ref(lidar_data));

    // Wait for threads to complete
    pose_thread.join();
    lidar_thread.join();

    // Display collected data in a clean batch format
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                     DATA BATCH SNAPSHOT                        ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << std::endl;

    // Display pose data
    std::cout << "\n┌──────────────── POSE DATA (Port " << (9000 + rover_id) << ") ────────────────┐" << std::endl;
    for (size_t i = 0; i < pose_data.size(); ++i) {
        std::cout << "\nPacket " << (i+1) << ":" << std::endl;
        print_raw_values_pose(&pose_data[i].packet);
        std::cout << "  Parsed: timestamp=" << std::fixed << std::setprecision(3) 
                  << pose_data[i].packet.timestamp << " sec" << std::endl;
        std::cout << "          position=(" << std::setprecision(2) 
                  << pose_data[i].packet.posX << ", " 
                  << pose_data[i].packet.posY << ", " 
                  << pose_data[i].packet.posZ << ")" << std::endl;
        std::cout << "          rotation=(" << std::setprecision(1) 
                  << pose_data[i].packet.rotXdeg << "°, " 
                  << pose_data[i].packet.rotYdeg << "°, " 
                  << pose_data[i].packet.rotZdeg << "°)" << std::endl;
    }
    std::cout << "└────────────────────────────────────────────────────────────────┘" << std::endl;

    // Display lidar data
    std::cout << "\n┌──────────────── LIDAR DATA (Port " << (10000 + rover_id) << ") ────────────────┐" << std::endl;
    for (size_t i = 0; i < lidar_data.size(); ++i) {
        std::cout << "\nPacket " << (i+1) << ":" << std::endl;
        print_raw_values_lidar(&lidar_data[i].packet.header);
        std::cout << "  Parsed: timestamp=" << std::fixed << std::setprecision(3) 
                  << lidar_data[i].packet.header.timestamp << " sec" << std::endl;
        std::cout << "          chunk " << (lidar_data[i].packet.header.chunkIndex + 1) 
                  << "/" << lidar_data[i].packet.header.totalChunks 
                  << ", " << lidar_data[i].packet.header.pointsInThisChunk << " points" << std::endl;
        
        // Show first 3 points
        if (lidar_data[i].packet.header.pointsInThisChunk > 0) {
            size_t points_to_show = std::min(uint32_t(3), lidar_data[i].packet.header.pointsInThisChunk);
            std::cout << "  Sample Points (first " << points_to_show << "):" << std::endl;
            for (size_t j = 0; j < points_to_show; ++j) {
                std::cout << "    Point " << j+1 << ": ["
                         << lidar_data[i].packet.points[j].x << ", "
                         << lidar_data[i].packet.points[j].y << ", "
                         << lidar_data[i].packet.points[j].z << "]" << std::endl;
            }
        }
    }
    std::cout << "└────────────────────────────────────────────────────────────────┘" << std::endl;

    std::cout << "\nData collection complete. " << pose_data.size() << " pose packets, " 
              << lidar_data.size() << " lidar packets collected." << std::endl;
    return 0;
}
