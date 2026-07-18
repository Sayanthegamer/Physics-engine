# Specification: Implement UI and Mesh Generation for Diverse Gears

## Overview
Add 3D mesh generation (procedural SDF) and UI controls for the new diverse gear types (Helical, Bevel, Worm, Rack, Internal). The underlying physics mathematics for these gears have already been implemented in a previous track.

## Functional Requirements
- **Mesh Generation:** Implement high-fidelity procedural SDF mesh generation for Helical, Bevel, Worm, Rack, and Internal gears. The visual fidelity must match the premium CAD-style look of the existing standard (Spur) gears.
- **UI Integration:** Add UI controls to instantiate these new gear types directly within the existing main toolbar, alongside the standard gears.
- **Parameter Adjustment:** Provide specific UI inputs for the unique parameters of these gears (e.g., helix angle for helical gears, pitch angle for bevel gears) to update the mesh in real-time.

## Non-Functional Requirements
- **Performance:** Must adhere to the extreme efficiency goals defined in `product.md`. The procedural mesh generation should run smoothly on budget hardware (e.g., Intel i3 10th Gen integrated graphics).
- **Style Consistency:** The UI must be implemented using the existing Dear ImGui framework, and rendering must utilize the Custom SDF Procedural GPU Rendering pipeline.

## Acceptance Criteria
- [ ] Users can select and add all five new gear types from the main toolbar.
- [ ] The generated meshes are visually accurate to their respective gear types and maintain high-fidelity aesthetics.
- [ ] Changing gear-specific parameters in the UI immediately updates the generated procedural mesh.
- [ ] Integrating these gears does not break the already implemented physics logic.

## Out of Scope
- Implementation of the physics/kinematic constraints for these gears (completed).
- Advanced material texturing or PBR rendering (staying with the current SDF approach).
