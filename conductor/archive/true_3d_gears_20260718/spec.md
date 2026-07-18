# Specification: True 3D Gear Mechanics & Global Positioning

## Overview
This track addresses the fundamental limitations in the current gear engine, which relies on a 2.5D constraint solver and simplified visuals. It introduces a true 3D kinematic solver for Worm, Helical, Rack, and Bevel gears, alongside a new Blender-style global 3D positioning system that applies to all objects.

## Functional Requirements

### 1. Global 3D Positioning System
- Implement a Blender-like 3D manipulation system (translate/rotate/scale) for all objects, replacing the current 2D-plane specific placement logic.
- Gears must be able to be positioned and oriented freely in 3D space.
- The constraint solver must evaluate gear positions dynamically to detect meshing based on bounding volumes and 3D proximity.

### 2. Gear Auto-Disconnect
- If a gear is moved away from its meshed partner (via the new 3D positioning system) beyond its meshing threshold, the physical constraint/coupling must be automatically severed.

### 3. Core Physics & Constraints (True 3D)
- Update `ConstraintSolver.hpp` to utilize fully defined 3D rotational axes (`axis_a`, `axis_b`) instead of hardcoding `.z` angular velocity.
- Introduce `LinearGearConstraint` for Rack gears to map linear velocity to angular velocity.

### 4. Gear-Specific Implementations
- **Worm Gear:** 
  - Generate a true threaded screw visual profile.
  - Enforce perfect locking (non-backdrivable): The worm can drive the gear, but the gear cannot drive the worm.
- **Helical Gear:** 
  - Introduce angle compatibility validation.
  - Strictly prevent placement/meshing if angles are mechanically incompatible (e.g., trying to mesh Left-Handed 30 deg with Left-Handed 30 deg).
- **Rack Gear:** 
  - Render a proper flat linear base with a 3D tooth profile.
  - Couple linear sliding motion to rotational motion in the solver.
- **Bevel Gear:** 
  - Render a precise conical frustum intersecting at a focal point.
  - Couple orthogonal 3D rotation axes in the solver.

## Non-Functional Requirements
- Maintain backward compatibility and stability for existing Spur gear logic.
- Ensure high performance of the 3D positioning/meshing checks to avoid frame drops during manipulation.

## Acceptance Criteria
- [ ] User can select any gear and move it using a Blender-like 3D gizmo/controls.
- [ ] Moving a gear away from a meshed gear instantly breaks their physical coupling.
- [ ] Bevel gears can be placed at 90-degree angles and rotate correctly in 3D space.
- [ ] Rack gear slides linearly when its coupled pinion rotates.
- [ ] Worm gear drives its partner but completely blocks backdriving from the partner.
- [ ] Helical gears refuse to couple if their twist angles/hands do not align.
- [ ] All new logic is thoroughly covered by unit tests.

## Out of Scope
- Full rigid-body dynamic collision detection (teeth colliding physically). We continue to rely on kinematic constraint solving for gear coupling.
