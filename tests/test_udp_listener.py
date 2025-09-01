#!/usr/bin/env python3
"""
Simple UDP listener to test if rover emulator is sending data
"""
import socket
import struct
import threading
import time

def listen_pose(rover_id):
    """Listen for pose packets on port 9000 + rover_id"""
    port = 9000 + rover_id
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(2.0)
    
    try:
        sock.bind(('127.0.0.1', port))
        print(f"Listening for pose data on port {port}...")
        
        for i in range(5):  # Listen for 5 packets
            try:
                data, addr = sock.recvfrom(1024)
                if len(data) >= 32:  # sizeof(PosePacket) = 8 + 6*4 = 32 bytes  
                    timestamp, posX, posY, posZ, rotX, rotY, rotZ = struct.unpack('dffffff', data[:32])
                    print(f"POSE: t={timestamp:.3f}, pos=({posX:.2f},{posY:.2f},{posZ:.2f}), rot=({rotX:.1f},{rotY:.1f},{rotZ:.1f})")
                else:
                    print(f"POSE: Received {len(data)} bytes (expected 28)")
            except socket.timeout:
                print(f"POSE: No data received on port {port}")
                break
    except Exception as e:
        print(f"POSE: Error on port {port}: {e}")
    finally:
        sock.close()

def listen_lidar(rover_id):
    """Listen for LiDAR packets on port 10000 + rover_id"""
    port = 10000 + rover_id
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(2.0)
    
    try:
        sock.bind(('127.0.0.1', port))
        print(f"Listening for LiDAR data on port {port}...")
        
        for i in range(3):  # Listen for 3 packets
            try:
                data, addr = sock.recvfrom(2048)
                if len(data) >= 20:  # sizeof(LidarPacketHeader) = 8 + 4*3 = 20 bytes
                    timestamp, chunk_idx, total_chunks, points_in_chunk = struct.unpack('dIII', data[:20])
                    print(f"LIDAR: t={timestamp:.3f}, chunk {chunk_idx+1}/{total_chunks}, {points_in_chunk} points")
                else:
                    print(f"LIDAR: Received {len(data)} bytes (expected >=20)")
            except socket.timeout:
                print(f"LIDAR: No data received on port {port}")
                break
    except Exception as e:
        print(f"LIDAR: Error on port {port}: {e}")
    finally:
        sock.close()

if __name__ == "__main__":
    rover_id = 1
    print(f"Testing rover {rover_id} emulator...")
    
    # Start listeners in separate threads
    pose_thread = threading.Thread(target=listen_pose, args=(rover_id,))
    lidar_thread = threading.Thread(target=listen_lidar, args=(rover_id,))
    
    pose_thread.start()
    lidar_thread.start()
    
    # Wait for threads to complete
    pose_thread.join()
    lidar_thread.join()
    
    print("UDP listener test complete.")
