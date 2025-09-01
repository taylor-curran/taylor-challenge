#ifndef UDP_PACKET_STRUCTURES_H
#define UDP_PACKET_STRUCTURES_H

#include <cstdint>  // For uint32_t, uint8_t types

// Maximum number of LiDAR points that can fit in one UDP packet
static const size_t MAX_LIDAR_POINTS_PER_PACKET = 100;

// Tell compiler: "Don't add padding between fields - we need exact binary layout"
#pragma pack(push, 1)

// Structure for rover position and orientation data
// Sent on UDP port 11003
struct PosePacket {
    double timestamp;    // Seconds since emulator started (8 bytes)
    float  posX;         // X position in units (4 bytes)
    float  posY;         // Y position in units (4 bytes)
    float  posZ;         // Z position in units (4 bytes)
    float  rotXdeg;      // Roll rotation in degrees (4 bytes)
    float  rotYdeg;      // Pitch rotation in degrees (4 bytes)
    float  rotZdeg;      // Yaw rotation in degrees (4 bytes)
};  // Total: 32 bytes

// Header for each LiDAR data chunk
struct LidarPacketHeader {
    double   timestamp;           // Same timestamp as PosePacket (8 bytes)
    uint32_t chunkIndex;         // Which chunk this is (0, 1, 2...) (4 bytes)
    uint32_t totalChunks;        // Total chunks for this scan (4 bytes)
    uint32_t pointsInThisChunk;  // How many points in this chunk (4 bytes)
};  // Total: 20 bytes

// Single LiDAR point in 3D space
struct LidarPoint {
    float x;  // X coordinate (4 bytes)
    float y;  // Y coordinate (4 bytes)
    float z;  // Z coordinate (4 bytes)
};  // Total: 12 bytes

// Complete LiDAR packet sent on UDP port 11002
struct LidarPacket {
    LidarPacketHeader header;                          // 20 bytes
    LidarPoint points[MAX_LIDAR_POINTS_PER_PACKET];   // 100 * 12 = 1200 bytes
};  // Total: 1220 bytes

// Vehicle telemetry (button states) sent on UDP port 11004
struct VehicleTelem {
    double  timestamp;     // Seconds since emulator started (8 bytes)
    uint8_t buttonStates;  // Bit field for button states (1 byte)
    // Bit 0: Button 1 (1 = pressed, 0 = not pressed)
    // Bit 1: Button 2
    // ... etc
};  // Total: 9 bytes

// Restore normal padding behavior
#pragma pack(pop)

#endif // UDP_PACKET_STRUCTURES_H
