Clarifying Questions for Planning
1. Architecture & Development Strategy
MVP vs Full: Should we build a minimal working version first (single rover, basic visualization) then iterate? Or architect for all 5 rovers from the start?
Data pipeline priority: Focus first on robust UDP ingestion or start with visualization using mock data?
2. Terrain Mapping Approach
Mesh generation strategy:
Height map (fast, assumes mostly flat terrain)?
Delaunay triangulation (more complex terrain)?
Voxel-based (handles overhangs/caves)?
Update frequency: Update terrain mesh every frame or batch updates?
Memory management: How much historical point data to retain?
3. Performance Optimization
Level of Detail (LOD): Should distant terrain have reduced detail?
Point cloud decimation: Downsample old/distant points to maintain performance?
GPU vs CPU processing: Where to do point-to-mesh conversion?
4. User Experience
Primary use case: Is this for real-time monitoring or post-mission analysis?
Camera modes: Fixed overview, rover-following, or free-fly priority?
Visual feedback: How to indicate data freshness, connection status, command acknowledgment?
5. Development Approach
Testing strategy:
Start with single rover emulator?
Create simplified test data files?
Mock UDP server for controlled testing?
Incremental milestones: What's the logical progression of features?
6. Risk Areas
Biggest technical risk: Is it the terrain mesh generation, the real-time performance, or the multi-rover coordination?
Fallback options: If terrain meshing is too slow, would a point cloud with color-by-height suffice initially?
7. Beyond Requirements
Nice-to-haves: Path trails for rovers? Measurement tools? Recording/playback?
Future extensibility: Should we structure for easy addition of more rovers or sensor types?