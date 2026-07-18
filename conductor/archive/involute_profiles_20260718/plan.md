# Implementation Plan: Fix Gear Tooth Clipping (Involute Profiles)

## Phase 1: Involute Profile Shader (SDF) Update
- [x] Task: Write Tests (Red Phase)
  - [x] Write unit tests for any CPU-side involute math utility functions.
- [x] Task: Implement to Pass Tests (Green Phase)
  - [x] Update the Gear rendering SDF shader to procedurally generate involute curves using the module, pressure angle, and teeth count.
- [x] Task: Phase Verification & Checkpoint (Refer to workflow.md) [checkpoint: manual_verified]

## Phase 2: Automatic Phase Alignment (Auto-Meshing)
- [x] Task: Write Tests (Red Phase)
  - [x] Write unit tests for the exact rotation offset calculation given two meshing gears of varying sizes.
- [x] Task: Implement to Pass Tests (Green Phase)
  - [x] Implement the rotation offset calculation logic in the physics/system layer.
  - [x] Update the gear placement interaction to automatically apply this offset when snapping to an existing gear.
- [x] Task: Phase Verification & Checkpoint (Refer to workflow.md) [checkpoint: manual_verified]

## Phase 3: UI Parameter Exposure
- [x] Task: Write Tests (Red Phase)
  - [x] Write tests ensuring the data layer (e.g., `EngineState` or `GearComponent`) accurately stores and updates `PressureAngle` and `Clearance`.
- [x] Task: Implement to Pass Tests (Green Phase)
  - [x] Add `PressureAngle` and `Clearance` properties to the relevant DOD data structure.
  - [x] Update the ImGui property panel to expose these fields with interactive sliders.
  - [x] Ensure the updated parameters are efficiently passed to the GPU shader for rendering.
- [x] Task: Phase Verification & Checkpoint (Refer to workflow.md) [checkpoint: manual_verified]
