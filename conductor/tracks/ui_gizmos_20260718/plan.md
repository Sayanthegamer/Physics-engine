# Implementation Plan: UI Interaction & Gizmos

## Phase 1: ImGuizmo Integration & UI Toolbar

- [ ] Task: Integrate ImGuizmo and Rendering Logic
  - [ ] Write tests verifying `GizmoMode` state transitions are accessible by UI context.
  - [ ] Implement `ImGuizmo` rendering loop in the main UI rendering pass, linked to the `selected_object_id`.
  - [ ] Map G, R, S keys to translate, rotate, and scale modes respectively.
- [ ] Task: UI Toolbar Panel
  - [ ] Write test verifying Toolbar UI component state (buttons for Translate/Rotate/Scale).
  - [ ] Implement the Toolbar window using Dear ImGui, ensuring it matches the sleek CAD aesthetic guidelines.
- [ ] Task: Phase Verification & Checkpoint (Refer to workflow.md)

## Phase 2: Auto-Snapping Logic

- [ ] Task: Snapping Proximity Detection
  - [ ] Write failing test for a function that detects when a dragged gear's 3D position is within snapping threshold of another gear.
  - [ ] Implement distance-based proximity detection using `pitch_radius` sums.
- [ ] Task: Phase Alignment & Constraint Creation
  - [ ] Write failing test to ensure the snapped gear calculates the correct rotational offset (`phi_b`) to align its teeth with the target gear.
  - [ ] Implement automatic `GearConstraint` creation when a snap is confirmed by dropping the gear.
- [ ] Task: Phase Verification & Checkpoint (Refer to workflow.md)
