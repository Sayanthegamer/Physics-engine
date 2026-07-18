# Implementation Plan: Diverse Gear Types

## Phase 1: Foundation and Data Structures
- [x] Task: Extend `RigidBodySoA` in `include/GearEngine.hpp`
  - [x] Write failing test for configuring generic gear parameters.
  - [x] Add `GearType` enum (`Spur`, `Helical`, `Bevel`, `Worm`, `Rack`, `Internal`).
  - [x] Add generic parameter arrays (e.g., `gear_params_0`, `gear_params_1`) to `RigidBodySoA`.
  - [x] Ensure `kMaxBodies` limits and strict DOD structure are maintained.
- [x] Task: Phase Verification & Checkpoint (Refer to workflow.md)

## Phase 2: Helical & Bevel Kinematics (Parallel & Intersecting Axes)
- [x] Task: Implement kinematic equations for Helical and Bevel gears
  - [x] Write failing test for transmission ratio calculation of Helical and Bevel meshes in `tests/PhysicsUtils_test.cpp` (or similar).
  - [x] Implement mathematical constraint logic connecting two rigid bodies configured as Helical or Bevel gears.
  - [x] Ensure all new unit tests pass.
- [x] Task: Phase Verification & Checkpoint (Refer to workflow.md)

## Phase 3: Worm, Rack, and Internal Kinematics (Specialized Constraints)
- [x] Task: Implement kinematic equations for Worm, Rack, and Internal gears
  - [x] Write failing test for transmission ratio calculation of Worm, Rack, and Internal meshes (handling the unique rotational-to-linear conversion for the Rack).
  - [x] Implement mathematical constraint logic for Worm, Rack, and Internal gears.
  - [x] Ensure all new unit tests pass.
- [x] Task: Phase Verification & Checkpoint (Refer to workflow.md)

## Phase: Review Fixes
- [x] Task: Apply review suggestions (No fixes required)
