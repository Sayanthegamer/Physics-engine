# Implementation Plan: MVP Core Engine Loop & Basic Physics

## Phase 1: Core Engine and Architecture Initialization
- [x] Task: Initialize CMake and fetch dependencies (GLFW, GLM, ImGui) b4bf8a4
- [x] Task: Setup `main.cpp` with basic GLFW window and OpenGL context b4bf8a4
- [x] Task: Implement `GearEngine.hpp` with `RigidBodySoA` and `EngineState` slot map b4bf8a4
- [x] Task: Phase Verification & Checkpoint (Refer to workflow.md) [checkpoint: b4bf8a4]

## Phase 2: Physics and Constraints
- [x] Task: Implement `CommandQueue` for deferred entity addition/removal b4bf8a4
- [x] Task: Implement `ConstraintSolver` structure b4bf8a4
- [x] Task: Add `GearConstraint` (ratio-based) and `MotorConstraint` solving logic b4bf8a4
- [x] Task: Integrate physics stepping into the main game loop b4bf8a4
- [x] Task: Phase Verification & Checkpoint (Refer to workflow.md) [checkpoint: b4bf8a4]

## Phase 3: In-Dev GUI and Interaction
- [x] Task: Setup Dear ImGui integration in the main loop b4bf8a4
- [x] Task: Implement `EditorCamera` for spatial navigation b4bf8a4
- [x] Task: Add raycasting logic to Z=0 plane for mouse interactions b4bf8a4
- [x] Task: Implement gear placement, snapping, and motor assignment UI b4bf8a4
- [x] Task: Implement `DebugRenderer` for instanced quad rendering b4bf8a4
- [x] Task: Phase Verification & Checkpoint (Refer to workflow.md) [checkpoint: b4bf8a4]

## Phase: Review Fixes
- [x] Task: Apply review suggestions (namespace fixes)
