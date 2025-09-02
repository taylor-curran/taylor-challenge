# Phase 1: Foundation TODOs

## Overview
**Goal**: Single rover visualization with real data streaming via UDP
**Approach**: Walking Skeleton - get minimal end-to-end pipeline working first
**Existing Assets**: rover_emulator binary, compressed data files, UDP packet specs

## Prerequisites
- [x] **P1.0** Extract rover data files
  - Run `make extract` to decompress .tar.xz files
  - Verify rover1.dat exists (~120MB uncompressed, not 360MB)
  - Test emulator: `./rover_emulator 1 --no-noise` (simplifies initial testing)
  
  **Completed Steps:**
  - Rebuilt emulator for macOS (was Linux binary): `make clean && make`
  - Extracted all 5 rover .dat files: `make extract`
  - Created test_udp_listener.cpp in tests/ to verify UDP streams
  - Confirmed data streaming: Pose (9001), LiDAR chunks (10001)
  - Run test: `make test && ./tests/test_udp_listener` (with emulator running)

## Step 1: UDP Data Reception (Foundation for all phases)
- [x] **P1.1** Create project structure ✅
  - `src/` - source files
  - `include/` - headers
  - `build/` - build artifacts
  - `CMakeLists.txt` - build configuration
  - Add dependencies: GLFW, OpenGL, GLM to CMake
  - **Completed**: CMake with GLFW 3.3, GLM 1.0.1, OpenGL
  - **Build command**: `make build-viz`
  - **Test passed**: `./build/bin/lidar_viz`

- [x] **P1.2** Define data structures matching emulator ✅
  ```cpp
  // Match binary layout from README.md lines 81-92, 105-122
  struct PosePacket { double timestamp; float posX/Y/Z; float rotX/Y/Zdeg; }
  struct LidarPacketHeader { timestamp, chunkIndex, totalChunks, pointsInThisChunk }
  struct LidarPoint { float x, y, z; }
  struct VehicleTelem { double timestamp; uint8_t buttonStates; }
  ```
  - Use `#pragma pack(push, 1)` for binary compatibility
  
  **Completed Steps:**
  - Created `include/udp_packet_structures.h` with all packet structures
  - Defined PosePacket (32 bytes) matching emulator binary layout
  - Defined LidarPacketHeader (20 bytes) and LidarPoint (12 bytes)
  - Defined LidarPacket with header + 100 points max (1220 bytes total)
  - Defined VehicleTelem for button states (9 bytes)
  - Used `#pragma pack(push, 1)` to ensure exact binary compatibility

- [x] **P1.3** Implement UDP receiver class ✅
  - Create `UDPReceiver` class with non-blocking socket
  - Bind to port 9001 (pose) for rover 1
  - Test: Receive and print PosePacket timestamps/positions
  
  **Completed Steps:**
  - Created `include/udp_receiver.h` with UDPReceiver class definition
  - Implemented `src/udp_receiver.cpp` with non-blocking socket support
  - Created `tests/test_pose_receiver.cpp` to validate pose packet reception
  - Successfully tested receiving pose packets at 10Hz from rover emulator
  - Verified correct packet structure (32 bytes) and data parsing

- [x] **P1.4** Add LiDAR chunk reassembly ✅
  - Bind second socket to port 10001 (LiDAR)
  - Create `LidarAssembler` class to buffer chunks by timestamp
  - Handle out-of-order chunks (use std::map<timestamp, chunks>)
  - Test: Print "Full scan received: N points" when all chunks arrive
  
  **Completed Steps:**
  - Created `include/lidar_assembler.h` with thread-safe chunk assembly
  - Implemented `src/lidar_assembler.cpp` with out-of-order chunk handling
  - Created `tests/test_lidar_assembler.cpp` to validate reassembly
  - Successfully tested receiving and assembling 10 chunks per scan (1000 points total)
  - Added cleanup for stale partial scans (2-second timeout)

- [x] **P1.5** Coordinate transformation pipeline
  - Create `Transform` class using GLM
  - Convert rover pose (degrees) to rotation matrix
  - Transform LiDAR points: rover-local → world coordinates
  - Formula: `world_point = pose_matrix * local_point`
  - Test: Print transformed points for verification

  **Status:** COMPLETED  
  **Priority:** High  
  **Dependencies:** P1.3, P1.4  

  **Goal:** Transform LiDAR points from rover-local to world coordinates.

  **Implementation Steps:**
  - [x] Create transformation matrices from pose data using GLM
  - [x] Apply transformations to LiDAR points
  - [x] Test with rover emulator data
  - [x] Verify transformed points match expected world positions

  **Completed Steps:**
  1. Created Transform class header (`include/transform.h`) with GLM-based transformation methods
  2. Implemented Transform class (`src/transform.cpp`) with:
     - `poseToMatrix()` - Creates transformation matrix from PosePacket
     - `createTransform()` - Creates transform from position and Euler angles
     - `transformPoint()` - Transforms single points from local to world coordinates
     - `transformLidarPoint()` - Transforms LiDAR points specifically
     - `transformLidarPoints()` - Batch transforms multiple points
     - Utility methods for extracting position/rotation from matrices
  3. Created comprehensive test program (`tests/test_transform.cpp`) that:
     - Tests basic transformations (identity, translation, rotation, combined)
     - Receives real-time pose and LiDAR data from rover emulator
     - Applies transformations to complete LiDAR scans
     - Validates world coordinate bounds and transformations
  4. Successfully tested with rover emulator:
     - Processed 5 complete scans with proper transformation
     - Verified world coordinates are consistent with rover pose
     - Confirmed bounding box calculations in world space

## Step 2: Basic 3D Window (Sets up rendering for Phase 2/3)
- [x] **P1.6** OpenGL/GLFW window setup ✅
  - 1280x720 window "Rover Visualization" 
  - OpenGL 4.1 core profile (macOS maximum)
  - Basic render loop at 60 FPS
  - Clear to dark gray background
  - Test rendering (triangle + dots) confirms pipeline works
  
  **Completed Steps:**
  - Created OpenGL window application in `src/main.cpp`
  - GLFW initialization with proper error handling and callbacks
  - OpenGL 4.1 Core Profile context (macOS compatibility)
  - 60 FPS render loop with frame timing and FPS display in title
  - ESC key handling and window resize callbacks
  - Test graphics rendering to verify OpenGL pipeline functionality
  - Successfully running on Apple M3 with Metal backend

## Step 2B: Clean up and verify basic visualization
- [x] **P1.7** Basic UDP → Render pipeline ✅
  - UDP thread receives pose/LiDAR packets on ports 9001/10001
  - LidarAssembler builds complete 1000-point scans from 5 packets
  - Transform class converts local LiDAR to world coordinates
  - Thread-safe data transfer with mutex protection
  - Successfully rendering red rover cube and cyan point cloud

- [ ] **P1.8** Fix camera and improve visibility
  - Adjust camera position to better view rover (starts ~168, 40, 186)
  - Make points larger (5-10 pixels) and brighter colors
  - Add depth testing to render order correctly
  - Remove all debug console output for cleaner execution

- [ ] **P1.9** Add coordinate reference helpers
  - Draw XYZ axes at origin (RGB = XYZ, 50 units long)
  - Draw grid on XZ plane (200x200m, 10m spacing, dim gray)
  - Draw rover path trail (last 100 positions)
  - Add simple text overlay: FPS and point count

## Step 3: Interactive camera controls (before shaders)
- [ ] **P1.10** Basic keyboard camera movement
  - WASD to move camera in XZ plane
  - QE to move up/down
  - Arrow keys to rotate view
  - R to reset camera to default position
  - Test thoroughly with rover moving around

- [ ] **P1.11** Mouse camera controls
  - Click and drag to orbit around focus point
  - Scroll wheel to zoom in/out
  - Right-click drag to pan view
  - Double-click to center on rover

- [ ] **P1.12** Camera presets
  - 1-5 keys for preset views (top, side, follow, orbit, free)
  - F key to toggle follow mode (camera follows rover)
  - Smooth transitions between presets

## Step 4: Improve rendering quality
- [ ] **P1.13** Point cloud improvements
  - Color by height gradient (blue→cyan→green→yellow→red)
  - Fade older points over time (alpha blending)
  - Keep history of last 10 scans visible
  - Add point size based on distance from camera

- [ ] **P1.14** Performance monitoring
  - Display render time in ms
  - Show UDP packet receive rate
  - Count dropped packets
  - Monitor point cloud size

## Step 5: Modern OpenGL migration (prepare for Phase 2)
- [ ] **P1.15** Shader infrastructure
  - Create basic vertex/fragment shader files
  - Load and compile shaders at startup
  - Create ShaderProgram class for management
  - Test with simple colored triangle

- [ ] **P1.16** Vertex Buffer Objects (VBO)
  - Convert point rendering to use VBOs
  - Dynamic buffer updates for streaming data
  - Benchmark performance vs immediate mode
  - Pre-allocate for 1M points

## Step 4: Performance & Robustness
- [ ] **P1.14** Timing measurements
  - Measure UDP receive → screen latency
  - Display in corner: "Latency: XXms"
  - Target: < 50ms end-to-end

- [ ] **P1.15** Error handling
  - Timeout detection: "Rover 1 disconnected" after 1 second
  - Graceful handling of partial LiDAR chunks
  - Log dropped packets to console (not UI yet)

- [ ] **P1.16** Memory monitoring
  - Track point cloud memory usage
  - Implement ring buffer eviction for old data
  - Display: "Memory: XX MB"

## Step 5: Prepare for Phase 2
- [ ] **P1.17** Abstract sensor interface
  - Create `ISensorProcessor` base class
  - `LidarProcessor : ISensorProcessor` implementation
  - `ProcessPacket()`, `GetPointCloud()` methods
  - Sets up for future sensor types

- [ ] **P1.18** Configuration system
  - Config file for: rover ID, window size, camera settings
  - Voxel grid parameters (even if not used yet)
  - Prepare for multi-rover port assignments

## Validation Checklist
- [ ] **V1** Run with real rover1 data (no-noise mode first)
- [ ] **V2** Verify points appear at correct world positions
- [ ] **V3** Camera controls feel responsive
- [ ] **V4** < 50ms latency achieved
- [ ] **V5** No memory leaks over 5-minute run

## What We're Leveraging
- **Existing emulator**: No need to parse .dat files directly
- **Port assignments**: Use rover_profiles.h structure (9001, 10001, etc.)
- **run_rovers.sh**: Can test with `./rover_emulator 1` individually
- **Binary packet formats**: Defined in README.md, match exactly

## What Sets Up Phase 2/3
- **Thread pools**: Already separating network/render threads
- **Flat arrays**: GPU-friendly data structures from start
- **ISensorProcessor**: Modular sensor architecture
- **Coordinate transforms**: World-space points ready for voxelization
- **Memory management**: Ring buffers scale to 5 rovers

## Notes
- Start with `--no-noise` flag for deterministic testing
- Focus on rover 1 only (ports 9001, 10001, 11001, 8001)
- Telemetry/buttons can wait until Phase 2
- Keep voxel grid allocation in mind (but don't implement yet)
