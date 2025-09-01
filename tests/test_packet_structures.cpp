#include <iostream>
#include <cassert>
#include "../include/udp_packet_structures.h"

int main() {
    std::cout << "Testing UDP packet structure sizes...\n\n";
    
    // Test PosePacket size
    std::cout << "PosePacket size: " << sizeof(PosePacket) << " bytes\n";
    std::cout << "  Expected: 32 bytes (8 + 6*4)\n";
    assert(sizeof(PosePacket) == 32);
    
    // Test LidarPacketHeader size
    std::cout << "\nLidarPacketHeader size: " << sizeof(LidarPacketHeader) << " bytes\n";
    std::cout << "  Expected: 20 bytes (8 + 3*4)\n";
    assert(sizeof(LidarPacketHeader) == 20);
    
    // Test LidarPoint size
    std::cout << "\nLidarPoint size: " << sizeof(LidarPoint) << " bytes\n";
    std::cout << "  Expected: 12 bytes (3*4)\n";
    assert(sizeof(LidarPoint) == 12);
    
    // Test LidarPacket size
    std::cout << "\nLidarPacket size: " << sizeof(LidarPacket) << " bytes\n";
    std::cout << "  Expected: 1220 bytes (20 + 100*12)\n";
    assert(sizeof(LidarPacket) == 1220);
    
    // Test VehicleTelem size
    std::cout << "\nVehicleTelem size: " << sizeof(VehicleTelem) << " bytes\n";
    std::cout << "  Expected: 9 bytes (8 + 1)\n";
    assert(sizeof(VehicleTelem) == 9);
    
    std::cout << "\n✅ All structure sizes are correct!\n";
    std::cout << "The #pragma pack(push, 1) is working properly.\n";
    
    return 0;
}
