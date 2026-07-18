# Implementation Plan: True 3D Gear Mechanics & Global Positioning

This plan outlines the execution roadmap based on the approved `spec.md`. It follows the Test-Driven Development (TDD) principles outlined in `workflow.md`.

## Phase 1: Global 3D Positioning & Auto-Disconnect

- [ ] Task: Scaffold 3D UI Context & Gizmo State
  - [ ] Write failing unit test for 3D placement UI context and axis rotation.
  - [ ] Implement 3D selection and gizmo state to pass the test.
  - [ ] Refactor and verify coverage.
- [ ] Task: Implement Auto-Disconnect logic
  - [ ] Write failing unit test simulating gears being moved apart beyond meshing threshold.
  - [ ] Implement distance threshold logic in `ConstraintSolver`/`main.cpp` loop to sever ties.
  - [ ] Refactor and verify coverage.
- [ ] Task: Phase Verification & Checkpoint (Refer to workflow.md)

## Phase 2: True 3D Physics Constraint Upgrades

- [ ] Task: Refactor ConstraintSolver to 3D axes
  - [ ] Write failing unit tests for 3D-axis rotations (e.g. testing Bevel gear orthogonal rotation).
  - [ ] Rewrite `SolveGears` in `ConstraintSolver.hpp` to utilize `axis_a` and `axis_b` vector math.
  - [ ] Refactor and verify coverage.
- [ ] Task: Introduce LinearGearConstraint (Rack logic)
  - [ ] Write failing test for Linear-to-Angular transmission ratio.
  - [ ] Implement `LinearGearConstraint` for Rack physics logic in the solver.
  - [ ] Refactor and verify coverage.
- [ ] Task: Phase Verification & Checkpoint (Refer to workflow.md)

## Phase 3: Gear-Specific Logic & Visual Rework

- [ ] Task: Worm Gear Implementation
  - [ ] Write failing tests for Worm gear non-backdrivability (perfect lock).
  - [ ] Update `GearMeshGenerator.cpp` for true threaded visual.
  - [ ] Implement solver locking logic for Worm gears.
  - [ ] Refactor and verify coverage.
- [ ] Task: Helical Gear Angle Compatibility
  - [ ] Write failing test for mismatched twist angles/hands rejecting coupling.
  - [ ] Implement strict angle validation logic before meshing.
  - [ ] Refactor and verify coverage.
- [ ] Task: Rack Gear & Bevel Gear Visuals
  - [ ] Write unit tests verifying new bounding boxes and placement states for Rack and Bevel gears.
  - [ ] Update `GearMeshGenerator.cpp` for a flat Rack base and precise Bevel conical frustum.
  - [ ] Refactor and verify coverage.
- [ ] Task: Phase Verification & Checkpoint (Refer to workflow.md)
