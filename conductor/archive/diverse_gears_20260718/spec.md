# Specification: Implement Diverse Gear Types

## Overview
This feature track aims to expand GearEngine's capabilities by introducing a variety of new gear types beyond standard involute spur gears. The new types include Helical, Bevel, Worm, Rack and Pinion, and Internal gears. This expansion will allow users to build much more complex mechanical systems.

## Functional Requirements
1. **Gear Types Supported:**
   - Helical gears (angled teeth, parallel axes)
   - Bevel gears (intersecting axes, conical)
   - Worm gears (high reduction, non-intersecting perpendicular axes)
   - Rack and pinion (linear motion conversion)
   - Internal gears (annular gears)
2. **Physics/Constraint Interactions:**
   - The initial implementation will focus on **kinematic constraints** (perfect mathematical meshing) rather than full rigid-body collision detection with contact points and impulses. This establishes the structural framework and mathematically accurate transmission of motion.
3. **Data Structures (DOD Alignment):**
   - Implement a `GearType` enum to categorize bodies.
   - Introduce generic parameter arrays (e.g., `gear_params_0`, `gear_params_1`) into `RigidBodySoA` to handle the specific geometric properties required by different gear types without breaking the strict Data-Oriented Design (DOD) paradigm or introducing polymorphism.

## Non-Functional Requirements
- **Performance:** Changes must not impact the extreme efficiency of the engine. The DOD layout must be preserved.
- **Scalability:** The addition of parameter arrays should be done carefully to ensure memory footprint for `RigidBodySoA` remains reasonable for up to `kMaxBodies = 8192`.

## Acceptance Criteria
- Engine state correctly stores and retrieves the `GearType` for each entity.
- The `RigidBodySoA` is capable of holding the parameters required to define Helical, Bevel, Worm, Rack and Pinion, and Internal gears.
- Kinematic equations for each gear type's transmission ratio and motion conversion (e.g., rotational to linear for rack and pinion) are correctly implemented.
- Unit tests verify the accurate calculation of output speeds/positions given input speeds/positions for all 5 new gear types under kinematic constraints.

## Out of Scope
- Full rigid body collision detection and impulse resolution for these new gears (to be handled in a future track).
- Visual rendering (meshes/SDF generation) for all these new gears, unless required for basic debugging.
