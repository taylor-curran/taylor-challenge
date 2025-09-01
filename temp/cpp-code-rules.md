# C++ Rover Visualization Project Rules

**Always do this**

-   **RAII only.** No raw `new/delete`, `malloc/free`, or owning raw pointers. Use values, `std::unique_ptr`, `std::shared_ptr` only when shared ownership is required. **Always close file descriptors/sockets in destructors or cleanup blocks.**
    
-   **Const & types.** Prefer `const`, `enum class`, and fixed-width ints (`std::uint32_t`). Avoid implicit narrowing; mark single-arg ctors `explicit`.

-   **Header hygiene.** Headers must be self-contained—include all dependencies explicitly. Never rely on transitive includes. Use include guards or `#pragma once`.
    
-   **Rule of 5/0.** If a type manages a resource, define/`=delete` copy/move ops intentionally; never rely on accidental defaults.
    
-   **Lifetimes.** No returning/caching references to temporaries or container elements that can reallocate. Be careful with `std::string_view`—only from stable storage.
    
-   **Errors.** Don’t throw through the hot path. Return status (`bool`/`enum`) + message, or use `expected`\-style patterns; mark fast-path functions `noexcept` when true.
    
-   **Concurrency.** Share data via messages/queues, not shared mutable state. Use SPSC/MPSC lock-free queues for network→process→render. Protect all shared state with `std::atomic` or locks (no data races).
    
-   **Thread Pool Architecture.**
    - Network thread pool: Handle all UDP receivers (ports 9001+, 10001+, 11001+ per rover)
    - Processing thread pool: LiDAR reassembly, coordinate transforms, voxelization  
    - Single render thread: OpenGL context ownership, never blocks
    - Lock-free SPSC between network→processing, MPSC for processing→render
    
-   **Timing.** Use `std::chrono::steady_clock` for all frame and latency budgets.
    

**Graphics & real-time**

-   **Threading.** Rendering must never block. No disk I/O, network I/O, or locks with unbounded wait in the render thread.
    
-   **Allocations.** No heap allocations in the render loop; preallocate, pool, or use ring buffers.
    
-   **OpenGL hygiene.** Wrap GL objects (VBO/VAO/FB/Programs) in RAII types; check errors in a debug context, never call `glGet*` per-frame; avoid `glFinish`.
    
-   **Throughput.** Prefer instanced draws and persistent mapped buffers; batch updates; minimize CPU↔GPU sync.
    

**Networking (UDP)**

-   **Non-blocking sockets.** Use `recvfrom` in non-blocking mode; handle `EWOULDBLOCK`.

-   **Socket configuration.** Always set `SO_REUSEADDR` for quick restarts. Set `SO_RCVBUF` to 4MB+ for high-throughput streams to reduce drop risk.
    
-   **Packets.** Treat each datagram atomically; validate byte count before parse; include a **version**, **sequence**, and **timestamp**; never assume in-order delivery.
    
-   **UDP Protocol (Project-Specific).** 
    - Rover N uses ports: 9000+N (pose), 10000+N (LiDAR), 11000+N (telemetry)
    - LiDAR chunk reassembly: Buffer by timestamp, handle out-of-order  
    - Track packet drop rate, log warnings at >5% loss
    - Timeout after 1 second of no packets = rover disconnect
    - Use `#pragma pack(1)` for binary compatibility with emulator
    
-   **Endianness & layout.** Use fixed-width types and explicit (de)serialization (no `reinterpret_cast` of packed structs across the wire).
    

**GLM / Math**

-   **GLM types.** Use GLM consistently (`glm::vec3`, `glm::mat4`); prefer column-major matrices for OpenGL; use `glm::value_ptr()` for GL buffer uploads.
    
-   **Copies vs moves.** Avoid copying large point clouds; pass by `const&`, move when transferring ownership.
    
-   **Coordinate sanity.** Keep a single world frame convention; document units; avoid hidden conversions.

**Spatial Data Structures**

-   **Hierarchical voxel grid.** Use sparse storage (`std::unordered_map` with spatial hash); pre-allocate for 500m×500m area (max 1km×1km).
    
-   **GPU-ready layout.** Keep voxel data in flat arrays for future GPU transfer; ring buffer for historical data (preserve 90% LOD); no pointer-based octrees in hot path.

**Sensor Extensibility**

-   **ISensorProcessor interface.** `ProcessPacket()`, `GetPointCloud()` methods; return data in world coordinates.
    
-   **Dynamic rover count.** No hardcoding N; use `std::unordered_map<RoverID, RoverState>`.
    
-   **Visualization hints.** Each sensor type defines color, size, render style.
    

**Pitfalls to actively prevent**

-   Dangling refs after `std::vector` reallocation; iterator invalidation.
    
-   Use-after-move; uninitialized reads; signed/unsigned mixups; integer overflow/shift UB.
    
-   Missing `virtual` dtor in polymorphic bases; object slicing; forgetting `override`.
    
-   Capturing stack refs in lambdas that outlive their scope.
    
-   Global mutable state / static initialization order fiasco.
    

**Performance guardrails**

-   **50ms end-to-end latency** target (UDP receive → screen update); 60 FPS minimum.
    
-   Network→process→render pipeline should complete in **<30ms**, leaving 20ms buffer.
    
-   Batch network→process→render with **double/triple buffering**; prefer SoA over AoS for hot loops.
    
-   **GPU-Ready Data.** Point clouds: separate position/color arrays; voxel grid: flat occupancy array + sparse index; avoid nested vectors; pre-allocate for 1M points.
    
-   Use `reserve()`/`shrink_to_fit()` intentionally; avoid `std::endl` (use `'\n'`).
    

**Build & tooling (non-negotiable)**

-   Compile flags: `-O2` (release) / `-g -O0` (dev). Use **target-scoped** warnings (`target_compile_options`) not global flags: `-Wall -Wextra -Wpedantic -Werror -Wconversion -Wshadow -Wnon-virtual-dtor`.
    
-   Sanitizers in CI/dev: Address + Undefined (`-fsanitize=address,undefined`), ThreadSanitizer for concurrency tests.
    
-   Static analysis: `clang-tidy` (cppcoreguidelines, performance, readability) must be clean or locally suppressed with rationale.
    
-   CMake: set C++17, treat warnings as errors, separate Debug/Release configs.

**Makefile Standards (Project-Specific)**

-   **File Organization:**
    - Visualization app: `src/*.cpp` → headers in `include/*.h` (CMake managed)
    - Test code: `tests/*.cpp` → builds to `build/bin/` for CMake tests
    - Emulator/tools: `emulator/*.cpp` → separate from main app (direct compilation)
    
-   **Build System (Hybrid Approach):**
    - **CMake**: Main visualization app (`make build-viz`)
    - **Direct g++**: Emulator and legacy tests (`make all`)
    
-   **Adding New Files:**
    - **For visualization app**: Add to `src/` or `include/`, CMake auto-discovers via GLOB
    - **For emulator**: Add to appropriate Makefile variable (`SRCS`, `TEST_SRCS`)
    - Always run `make cmake-clean && make build-viz` after adding files to verify
    
-   **Build Targets:**
    - `make all`: Build emulator + extract data (legacy)
    - `make build-viz`: Build visualization app via CMake
    - `make cmake-config`: Configure CMake (auto-called by build-viz)
    - `make cmake-clean`: Clean CMake build directory
    - `make test`: Build legacy test executables
    - `make clean`: Remove emulator/test binaries (NOT CMake build/)
    - Keep ALL binaries OUT of git (use .gitignore)
    
-   **Dependencies:**
    - CMake handles dependencies for visualization app automatically
    - External libs in CMakeLists.txt: GLFW 3.3+, GLM 1.0.1+, OpenGL
    - Emulator has no external dependencies (self-contained)

### How your this rule prevents the common C++ nasties

-   **No garbage collector?** You’re using **RAII everywhere** + Rule of 0/5 ⇒ deterministic destruction of memory, sockets, files, GL objects, etc.
    
-   **Memory leaks**: ban on owning raw pointers, use of `unique_ptr`, resource wrappers for GL/FDs, and “no allocations in the render loop” reduce leaks and **fragmentation**.
    
-   **Dangling refs / UAF**: cautions on `string_view`, iterator invalidation, move-after-use, and container reallocation address classic lifetime bugs.
    
-   **Double free / mismatched delete**: eliminated by containers/RAII (no `new[]/delete[]`).
    
-   **Thread bugs**: queues/atomics + TSan requirement covers data races (a huge source of “heisenbugs” and corrupt heaps).
    
-   **ABI/UB landmines**: fixed-width types + explicit serialization for UDP avoid UB from padding/endianness; Eigen/PCL alignment notes prevent misaligned new.
    

### Hardening (Phase 2+)

-   **Sanitizers** (Phase 2): In Debug/CI, build with  
    `-fsanitize=address,undefined,leak -fno-omit-frame-pointer`  
    (ASan/UBSan/LSan). Treat *any* sanitizer finding as a PR blocker.
    
-   **Leak checks** (Production only): Long-run with Valgrind or LSan for release candidates.
    
-   **`shared_ptr` cycles**: Ban owning cycles; require `weak_ptr` for back-refs. Add a checklist item: “Could this graph create a cycle?”
    
-   **Debug iterators** (dev only): `_GLIBCXX_ASSERTIONS` or `-D_GLIBCXX_DEBUG` to catch bounds/iterator misuse early.
    
-   **Scope guards**: Use a `scope_exit` helper (e.g., tiny local utility or `absl::Cleanup`/`boost::scope_exit`) for multi-step setup to stay exception-safe.
    
-   **GPU/GL lifetime**: All `gl*` handles must live in RAII wrappers and **be destroyed on the GL thread**; deletion is part of DoD.
    
-   **Backpressure**: All inter-thread queues must be **bounded**; unbounded queues = “infinite RAM leak.” Define drop/overwrite policy explicitly.
    
-   **Thread shutdown**: All worker threads must `join()` before main exits (no “leaks at exit”); failing to join is a CI failure.
    

### Quick rubric to answer “are we safe?”

-   **Ownership** clear? (values/`unique_ptr`/RAII only)
    
-   **No hot-path allocations** or blocking in render?
    
-   **Sanitizers clean** (ASan/UBSan/LSan/TSan)?
    
-   **No cycles** with `shared_ptr`?
    
-   **Queues bounded** and lifetimes joined on shutdown?
    
-   **GL/FDs** released by RAII on the right thread?
    