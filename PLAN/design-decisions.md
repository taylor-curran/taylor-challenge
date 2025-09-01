# Design Decisions Summary

## Core Architecture

### Threading Model
**Decision**: Thread pools (Option C)
- Networking thread pool for UDP reception
- Processing thread pool for data transformation
- Render thread for visualization
- **Rationale**: Supports different user modes (remote driving, automation) and scales better than single-threaded or thread-per-rover

### Development Approach
**Decision**: Walking Skeleton
- Build minimal end-to-end pipeline first
- Single rover → points on screen → iterate
- Add features incrementally
- **Rationale**: Avoids integration issues, allows continuous sanity checking

## Spatial Data Structures

### Terrain Representation
**Decision**: Hierarchical Grid with voxel-based mapping
- Operational area: 500m × 500m (max 1km × 1km)
- Multiple resolution levels for zooming
- Fallback: Delaunay triangulation if voxels prove too complex

### Data Persistence
- Update terrain every frame
- Assume historical data valid until proven otherwise
- 90% LOD retention
- Preserve all historical data

## Processing Pipeline

### Data Flow
```
Raw Points → Filter/Downsample → Voxelize → Extract Surface → Render
                ↓                    ↓            ↓
            (Noise?)          (Occupancy?)  (Marching Cubes?)
```

### Data Fidelity
**Priority**: Accurate representation over smoothness
- Start with highest granularity
- Include more raw data initially
- Reduce noise/granularity only if needed
- **Rationale**: Conservative approach, can always filter later

## Performance & Reliability

### Performance Priorities
1. Real-time performance (sub-50ms)
2. Multi-rover coordination
3. Mesh generation
- Point cloud rendering as valid fallback

### Error Handling
- **UDP drops**: Track and log drop rate, raise warning if excessive (tracking method to be determined in Phase 3)
- **Out-of-order packets**: Should not be issue if performant
- **Rover disconnect**: Timeout after N seconds, raise error
- **Memory**: Monitor utilization continuously

## User Interface

### Camera Modes
1. Free-fly mode (implement first - simpler)
2. Rover-follow mode (implement second)
3. Data freshness overlay (toggle feature)

### Development Priority
- Get something on screen ASAP
- Visual feedback before comprehensive testing
- UI sanity check before deep implementation

## Implementation Details

### GPU Readiness
**Current Version (CPU-only)**:
- Use flat arrays for data storage (std::vector<float> not nested structures)
- Avoid linked lists and pointer-heavy data structures
- Organize memory for cache-friendly access patterns
- Keep transformation matrices in column-major order

**Future GPU Transition Preparation**:
- Design data structures to be easily copied to GPU buffers
- Separate data from logic (POD structs where possible)
- Use batch operations that map well to compute shaders
- Abstract rendering pipeline to allow OpenGL → Vulkan/CUDA swap

### Modularity Requirements
- Dynamic rover count (no hardcoding N=5)
- **Sensor Extensibility Architecture**:
  - `ISensorProcessor` base interface with standard methods:
    - `ProcessPacket(const UDP_Packet&)` - parse sensor-specific data
    - `GetPointCloud()` - return 3D points in world coordinates
    - `GetVisualizationHints()` - color, size, render style preferences
  - Sensor registry that maps packet types → processor instances
  - Example future sensors: cameras (texture mapping), GPS (waypoints), temperature (heatmaps)
  - Each sensor type can define its own visualization layer
- Plugin architecture allows runtime loading of new sensor modules

## Incremental Milestones

### Phase 1: Foundation
1. UDP data pipeline for single rover
2. Basic 3D window with free-fly camera
3. Point cloud rendering

### Phase 2: Core Features
4. Hierarchical voxel terrain system
5. UI controls with ImGui

### Phase 3: Scale & Polish
6. Multi-rover support
7. Performance optimization
8. Sensor plugin architecture

## Stretch Goals
- Path trails for rovers
- Recording/playback functionality
- Mock UDP server for testing
- Track UDP drop rate and raise warning if excessive

## Technical Stack (from target-tech-stack.md)
- **Language**: C++17
- **Graphics**: OpenGL 4.3+ with GLFW
- **UI**: Dear ImGui
- **Math**: GLM
- **Networking**: POSIX UDP sockets
- **Build**: CMake

## Minimum Success Criteria
- Single rover visualization working with real data
- Sub-50ms latency achieved
- Modular architecture supports N rovers
- Terrain persistence across time
- **Extensible sensor framework demonstrated** (at least 2 sensor types: LiDAR + telemetry)
