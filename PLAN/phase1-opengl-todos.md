# Phase 1: OpenGL Incremental Development Plan

## Core Philosophy
- **Small wins**: Each step should take < 30 minutes and produce visible results
- **Modern OpenGL only**: Use VAO/VBO/Shaders from the start (OpenGL 3.3+)
- **Debug everything**: Console output + on-screen text for every value we care about
- **Real data early**: Use actual LiDAR packets but start with just ONE packet
- **Understand before advancing**: Don't move to next step until current step is clear

## Step 1: Modern OpenGL Foundation (No LiDAR Yet)

### GL-1.1: Minimal Modern OpenGL Window
- [ ] Create `gl_test_01_window.cpp`
- [ ] GLFW window 1280x720, OpenGL 3.3 Core Profile
- [ ] Clear to dark blue background
- [ ] Print OpenGL version to console
- [ ] ESC to exit
- [ ] **Success**: Blue window appears, console shows "OpenGL 3.3"

### GL-1.2: First Shader Triangle
- [ ] Create basic vertex & fragment shaders (hardcoded in strings)
- [ ] Compile and link shaders with error checking
- [ ] Create VAO and VBO for a single triangle
- [ ] Draw white triangle in center of screen
- [ ] **Success**: White triangle on blue background

### GL-1.3: Colored Points Instead of Triangle
- [ ] Modify to render 3 points instead of triangle
- [ ] Use GL_POINTS with glPointSize(10.0f)
- [ ] Pass color as vertex attribute
- [ ] Make points red, green, blue
- [ ] **Success**: Three colored dots on screen

### GL-1.4: Debug Overlay Text
- [ ] Add simple immediate-mode text rendering (can use deprecated for overlay only)
- [ ] Display "FPS: XX" in corner
- [ ] Display "Points: 3" 
- [ ] Display mouse position
- [ ] **Success**: Debug info visible on screen

## Step 2: Real LiDAR Data - Static Visualization

### GL-2.1: Read ONE LiDAR Packet
- [ ] Create `test_data_loader.cpp` helper
- [ ] Read exactly ONE LidarPacket from UDP or file
- [ ] Print all 100 points to console (x,y,z values)
- [ ] Understand coordinate ranges (are they in meters? millimeters?)
- [ ] **Success**: Console shows 100 point coordinates

### GL-2.2: Render ONE Packet as Points
- [ ] Load the 100 points into VBO
- [ ] Center points around origin (subtract mean position)
- [ ] Scale if needed (divide by 1000 if in mm?)
- [ ] Render as white dots, size 5.0f
- [ ] **Success**: See roughly 100 dots forming some shape

### GL-2.3: Analyze the Shape
- [ ] Add bounding box visualization (wireframe cube around points)
- [ ] Print min/max coordinates to console
- [ ] Color points by height (blue=low, red=high)
- [ ] Add coordinate axes (RGB = XYZ)
- [ ] **Success**: Can identify what surface the LiDAR is seeing

### GL-2.4: Orthographic View First
- [ ] Use orthographic projection (no perspective distortion)
- [ ] Top-down view (looking down Y axis)
- [ ] Add grid lines every 10 units
- [ ] Display "Scale: 1 unit = 1 meter" (or whatever it is)
- [ ] **Success**: Clear 2D top-down view of point pattern

## Step 3: Multiple Packets & Patterns

### GL-3.1: Accumulate 10 Packets
- [ ] Read 10 sequential LidarPackets
- [ ] Display "Packets: 10, Points: 1000"
- [ ] Different color per packet (to see grouping)
- [ ] Print timestamp of each packet
- [ ] **Success**: See 10 distinct colored groups

### GL-3.2: Understand Scan Pattern
- [ ] Animate through packets (spacebar = next packet)
- [ ] Show only current packet
- [ ] Display "Packet 3/10, Timestamp: X.XXX"
- [ ] Identify rotation pattern in data
- [ ] **Success**: See how LiDAR sweeps/rotates

### GL-3.3: Full 360° Scan
- [ ] Read enough packets for one complete rotation
- [ ] Detect when rotation completes (angle wraps)
- [ ] Display "Full scan: 50 packets, 5000 points"
- [ ] Color by angle instead of packet
- [ ] **Success**: See complete 360° scan pattern

## Step 4: 3D Perspective & Camera

### GL-4.1: Switch to Perspective Projection
- [ ] Add perspective projection matrix
- [ ] Initial camera at (0, -50, -100) looking at origin
- [ ] Keep orthographic as toggle option (O key)
- [ ] Display "Mode: Perspective" on screen
- [ ] **Success**: Points have depth perception

### GL-4.2: Simple Camera Controls
- [ ] WASD = move camera XZ
- [ ] QE = move camera up/down
- [ ] Mouse drag = rotate view
- [ ] R = reset camera
- [ ] Display camera position on screen
- [ ] **Success**: Can orbit around point cloud

### GL-4.3: Focus on Point Cloud
- [ ] Auto-calculate bounding box of all points
- [ ] F key = frame/focus on points
- [ ] Adjust camera distance based on cloud size
- [ ] Display "Cloud size: X x Y x Z meters"
- [ ] **Success**: Points always nicely framed

## Step 5: Add Rover & Coordinate Systems

### GL-5.1: Show Rover Origin
- [ ] Add red cube at (0,0,0) as rover
- [ ] Size cube appropriately (1x1x1 meter?)
- [ ] Add arrow showing rover forward direction
- [ ] Label "ROVER" above it
- [ ] **Success**: Clear rover representation

### GL-5.2: Rover-Relative Coordinates
- [ ] Display points in rover-local frame
- [ ] Show rover axes (forward=red, right=green, up=blue)
- [ ] Toggle between rover-local and world coords (L key)
- [ ] Display "Coordinate Frame: Local"
- [ ] **Success**: Understand LiDAR mounting position

### GL-5.3: Read Pose Data
- [ ] Read one PosePacket from UDP
- [ ] Display position (X,Y,Z) on screen
- [ ] Display rotation (Roll,Pitch,Yaw) in degrees
- [ ] Move rover cube to pose position
- [ ] **Success**: Rover at correct world position

## Step 6: Transformations

### GL-6.1: Manual Transform Test
- [ ] Hardcode a test transform (translate by 10,0,0)
- [ ] Apply to all LiDAR points
- [ ] Toggle transform on/off (T key)
- [ ] Show "Transform: ON/OFF"
- [ ] **Success**: Points shift when toggled

### GL-6.2: Rotation Transform Test  
- [ ] Add 45° rotation around Y axis
- [ ] See points rotate with rover
- [ ] Display rotation matrix values
- [ ] Verify with known test points
- [ ] **Success**: Rotation looks correct

### GL-6.3: Full Pose Transform
- [ ] Build transform matrix from PosePacket
- [ ] Transform LiDAR points to world space
- [ ] Show world axes (different from rover axes)
- [ ] Display "World Frame" vs "Rover Frame"
- [ ] **Success**: Points in correct world position

## Step 7: Live Data Streaming

### GL-7.1: Continuous Packet Reading
- [ ] Read packets in separate thread
- [ ] Display "Packets/sec: XX"
- [ ] Show data rate in MB/s
- [ ] Implement ring buffer for points
- [ ] **Success**: Smooth data flow

### GL-7.2: Synchronized Pose+LiDAR
- [ ] Match LiDAR packets to nearest pose
- [ ] Display timestamp difference
- [ ] Highlight unsynchronized data in red
- [ ] Show "Sync delay: XX ms"
- [ ] **Success**: Proper time alignment

### GL-7.3: Moving Rover Visualization
- [ ] Show last N seconds of data
- [ ] Trail of points behind moving rover
- [ ] Fade older points
- [ ] Display "History: 5 seconds"
- [ ] **Success**: See rover path via point trail

## Success Metrics for Each Step
- Can explain what every line of code does
- Can predict what will appear on screen before running
- Debug output confirms our understanding
- Each step builds on previous without breaking it

## Debug Dashboard (Always Visible)
```
┌─────────────────────────────┐
│ FPS: 60                     │
│ Points: 5000                │
│ Packets: 50                 │
│ Camera: (0, -50, -100)      │
│ Mode: Perspective           │
│ Frame: World                │
│ Rover: (10.5, 0.0, 5.2)    │
│ Data Rate: 1.2 MB/s        │
└─────────────────────────────┘
```

## When You Get Stuck
1. Go back to last working step
2. Add more console output
3. Reduce data to minimum (1 point, 1 packet)
4. Draw debug geometry (axes, boxes, lines)
5. Check coordinate system conventions
6. Verify matrix multiplication order
