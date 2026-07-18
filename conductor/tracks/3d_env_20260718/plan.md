# Implementation Plan: 3D Gears & Workspace Environment

## Phase 1: Procedural 3D Gear Generation
- [x] Task: Create tests for 3D Gear Mesh generator math and data structures
- [x] Task: Implement `GearMeshGenerator` to procedurally build cylinder/teeth vertices and indices
- [x] Task: Update the engine rendering loop to support drawing the 3D meshes for gears
- [x] Task: Map the physics `GearConstraint` / `RigidBody` transforms to the 3D gear meshes
- [x] Task: Refactor and optimize mesh generation for DOD (Structure of Arrays caching)
- [x] Task: Phase Verification & Checkpoint (Refer to workflow.md)

## Phase 2: Workspace Environment (Grid & Floor)
- [x] Task: Write tests for Infinite Grid and Floor configuration states
- [ ] Task: Implement rendering logic for a light infinite grid on the XZ plane
- [ ] Task: Implement rendering logic for a finite solid floor plane
- [ ] Task: Integrate grid and floor rendering into the main render pass
- [ ] Task: Phase Verification & Checkpoint (Refer to workflow.md)

## Phase 3: ImGui 'View' Settings Integration
- [ ] Task: Write tests for ImGui view state management (toggles for grid, floor, wireframe)
- [ ] Task: Create a new ImGui "View" settings window in `EditorCamera` or a dedicated UI class
- [ ] Task: Wire the UI toggles to the respective rendering passes (Grid, Floor, Wireframe mode)
- [ ] Task: Validate that UI captures mouse/keyboard correctly without interacting with the scene
- [ ] Task: Phase Verification & Checkpoint (Refer to workflow.md)
