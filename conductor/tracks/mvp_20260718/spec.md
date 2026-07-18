# Specification: MVP Core Engine Loop & Basic Physics

## Overview
This track establishes the foundational architecture of GearEngine. It implements the core game loop, the Data-Oriented Design (DOD) structures for rigid bodies, basic constraint solving, and an initial "in-development" UI for placing and interacting with gears.

## Functional Requirements
- **Core Loop:** Initialize GLFW and OpenGL context. Run a fixed-timestep physics loop integrated with a dynamic rendering loop.
- **DOD Architecture:** Implement `RigidBodySoA` (Structure of Arrays) and `EngineState` with generational slot map handles for O(1) allocation and removal.
- **Physics & Constraints:**
  - Implement a `ConstraintSolver` using sequential impulses.
  - Support `GearConstraint` (ratio-based rotational linking) and `MotorConstraint` (driving rotation).
- **Deferred Commands:** Use a `CommandQueue` to handle entity creation/destruction safely before physics ticks.
- **In-Dev GUI & Interaction:**
  - ImGui-based statistics panel (FPS, Active Bodies, Motor Controls).
  - Mouse-based 3D interaction (raycasting to Z=0 plane).
  - Ability to place gears, auto-snap to existing gears, assign motors, and delete gears.
- **Rendering:** Use `DebugRenderer` to draw instanced quads representing gears (preparing for procedural SDF rendering).

## Out of Scope (For this Track)
- Advanced collisions (rigid body penetration resolution).
- Complex UI skinning or "final" modern aesthetics.
- 3D mesh loading (e.g., OBJ files).
