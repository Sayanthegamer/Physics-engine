# GearEngine: DOD Mechanical Physics

A performance-focused, standalone 3D mechanical physics engine built from scratch using a strict Data-Oriented Design (DOD) paradigm in C++20.

## Overview
GearEngine is designed for high-fidelity, deterministic mechanical simulations (gears, cogs, pipes, automated wire routing) that run smoothly on budget hardware without sacrificing a premium user interface.

### Key Architectural Decisions

1. **Strict Data-Oriented Design (No V-Tables)**
   - *Why:* To maximize CPU L1/L2/L3 cache efficiency and guarantee bit-exact determinism. Generic base `Constraint` classes with virtual methods cause cache misses and unpredictable instruction branching.
   - *How:* We use distinct, flat contiguous arrays for each constraint type (e.g., `std::vector<GearConstraint>`).

2. **Structure of Arrays (SoA) for Shared RigidBody State**
   - *Why:* Grouping all positions, velocities, and torques into flat parallel arrays allows SIMD vectorization and ensures constraint solvers always operate on perfectly tightly packed data. 
   - *How:* `RigidBodySoA` is pre-allocated to a fixed `kMaxBodies` capacity to avoid vector reallocation and pointer invalidation.

3. **Generational Slot Map Handles**
   - *Why:* When simulating dynamic subsystems (mid-simulation insertion and removal), pointers to bodies can become stale. We need O(1) detection of dead bodies without reference counting overhead.
   - *How:* Entities are referenced by an `EntityHandle` containing an `index` and a `generation`. Removing a body increments its generation, instantly invalidating all existing handles to it.

4. **Deferred Command Queue**
   - *Why:* To enforce symmetry and avoid interrupting the CPU physics step mid-flight. State changes must happen at controlled boundaries.
   - *How:* A `CommandQueue` using `std::variant` captures intent (Add, Remove) and processes them all right before the `dt` accumulator ticks the physics step.

5. **Sequential Impulse Solver**
   - *Why:* The Box2D-style iterative sequential impulse solver is robust for analytical constraints (perfect gear teeth synchronization without slipping) and handles warm-starting natively.

## Getting Started

### Prerequisites
- CMake 3.20+
- A C++20 compatible compiler
- Internet connection (for the first build to fetch GLFW, Dear ImGui, and GLM).

### Build Instructions
```bash
# Configure the project
cmake -B build

# Build the executable
cmake --build build --config Release
```

The executable will be located in the `build/` directory.

### Build Considerations (MSVC)
- **C++ Exceptions (`/EHsc`)**: Although the engine core heavily leverages DOD, we utilize `std::variant` for the deferred `CommandQueue`. Because `std::variant` internally requires exception unwind semantics, MSVC requires the `/EHsc` compile flag.
- **Strict Warnings (`/WX`)**: We compile with warnings-as-errors. During development, placeholder variables (like unused `bodies` references in the constraint solver) must be explicitly commented out in the parameter list (e.g., `RigidBodySoA& /*bodies*/`) to prevent C4100 unreferenced parameter build failures.

---
*For a deeper dive into the specific systems, please review `docs/ARCHITECTURE.md`.*
