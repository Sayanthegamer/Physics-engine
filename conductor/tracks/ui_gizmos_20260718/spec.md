# Specification: UI Interaction & Gizmos

## Overview
This track focuses on wiring up the backend 3D spatial logic (Translate, Rotate, Scale) developed in the previous track to a fully interactive frontend User Interface. The goal is to provide a premium, CAD-style interaction model that is intuitive and accessible, using existing libraries for rendering standard gizmos.

## Functional Requirements
1. **Gizmo Rendering:**
   - Integrate the `ImGuizmo` library to handle 3D gizmo rendering (arrows for translation, rings for rotation, cubes for scale).
   - Render the active gizmo centered on the `selected_object_id` within the 3D viewport.

2. **Interaction Mode Switching:**
   - Implement Blender-style keyboard shortcuts:
     - `G`: Grab (Translate)
     - `R`: Rotate
     - `S`: Scale
   - Provide a dedicated UI Toolbar panel for non-power users to switch between these modes via clickable buttons.

3. **Auto-Snapping (Meshing):**
   - When a gear is translated close to another gear, the system must automatically snap the dragged gear into a meshed position.
   - Snapping distance and position must be calculated based on the sum of the `pitch_radius` of the two gears.
   - The snapped gear must automatically orient its teeth (calculate the required rotation offset) to prevent collision clipping with the target gear's teeth.

## Non-Functional Requirements
- **Performance:** ImGuizmo integration must not negatively impact the main SDF render loop or physics solver rate.
- **Aesthetics:** The toolbar must match the sleek, modern styling defined in the product guidelines.

## Acceptance Criteria
- A user can select a gear in the viewport and see a 3D gizmo appear over it.
- Pressing G, R, S changes the gizmo type.
- Clicking and dragging the gizmo arrows translates the gear in 3D space.
- Dragging a gear near another gear automatically snaps them together at exactly the correct meshing distance and phase angle.

## Out of Scope
- Advanced multi-selection gizmo operations (e.g., translating 5 gears at once).
- Custom procedural rendering of gizmos (using ImGuizmo instead).
