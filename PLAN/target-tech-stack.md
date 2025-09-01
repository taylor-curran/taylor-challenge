# Tech Stack

## Architecture
Split backend/frontend for production scalability, but deliverable as monolithic app for challenge.

## Backend (Data Processing)
- **Language**: C++17
- **Networking**: POSIX UDP sockets (rover communication)
- **Point Cloud**: Custom lightweight implementation (Phase 1), PCL for advanced processing (Phase 2+)
- **Math**: GLM (OpenGL Mathematics)
- **API**: gRPC for frontend communication (future)
- **Build**: CMake

## Frontend (Visualization)
- **Language**: C++ 
- **Window**: GLFW
- **Graphics**: OpenGL 4.3+
- **UI**: Dear ImGui (overlay)
- **IPC**: Shared memory (local) or gRPC (remote)
- **Rationale**: Separate UI enables multiple operator interfaces (web, mobile, VR) without touching core processing

## Data Flow
```
Rovers --UDP--> Backend --Process--> Frontend --Render--> Display
                   |                     |
              (Point clouds)         (3D Scene)
              (Terrain mesh)         (UI Controls)
              (Rover states)         (Commands)
```

## Performance Targets
- End-to-end latency: <50ms
- Point cloud processing: 1M points/sec
- Render: 60 FPS minimum
- Platform: Ubuntu 22.04, ThinkPad T16 Gen 3

## Patterns
- Entity-Component System (ECS) for extensibility (Phase 2+)
- Scene graph for spatial hierarchy
- Double buffering for smooth updates
- Thread separation: network, processing, rendering
