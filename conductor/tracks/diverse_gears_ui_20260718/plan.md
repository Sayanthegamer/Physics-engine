# Implementation Plan: Implement UI and Mesh Generation for Diverse Gears

## Phase 1: Diverse Gears UI Toolbar Integration
- [x] Task: Add UI controls for Helical, Bevel, Worm, Rack, and Internal gears
  - [x] Write failing test for new gear type UI instancing and event emission.
  - [x] Implement UI buttons/dropdowns in the main toolbar using ImGui.
  - [x] Ensure tests pass when UI components correctly dispatch creation events.
- [x] Task: Phase Verification & Checkpoint (Refer to workflow.md)

## Phase 2: Procedural SDF Mesh Generation (Helical & Bevel)
- [x] Task: Implement SDF generation for Helical and Bevel gears
  - [x] Write failing test for SDF evaluation of Helical and Bevel geometries.
  - [x] Implement the shader/SDF functions for Helical and Bevel gears, ensuring CAD-style visual fidelity.
  - [x] Ensure SDF generation tests pass.
- [~] Task: Phase Verification & Checkpoint (Refer to workflow.md)

## Phase 3: Procedural SDF Mesh Generation (Worm, Rack, Internal)
- [ ] Task: Implement SDF generation for Worm, Rack, and Internal gears
  - [ ] Write failing test for SDF evaluation of Worm, Rack, and Internal geometries.
  - [ ] Implement the shader/SDF functions for these gears.
  - [ ] Ensure SDF generation tests pass.
- [ ] Task: Phase Verification & Checkpoint (Refer to workflow.md)

## Phase 4: Parameter Adjustment UI and Real-Time Linking
- [ ] Task: Create specific UI input fields for gear parameters (e.g., helix angle, pitch angle)
  - [ ] Write failing test for parameter update events and data binding.
  - [ ] Implement ImGui input fields dynamically based on the selected gear type.
  - [ ] Link UI parameters to the real-time SDF mesh generation pipeline.
  - [ ] Ensure tests pass for parameter updates.
- [ ] Task: Phase Verification & Checkpoint (Refer to workflow.md)
