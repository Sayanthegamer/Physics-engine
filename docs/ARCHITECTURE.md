# System Architecture Specification

This document details the core systems implemented in Milestone 1 of the GearEngine.

## 1. The Fixed Timestep Loop (`main.cpp`)

The engine operates on a strictly decoupled loop to guarantee determinism regardless of the user's framerate.
- **`kFixedDt = 1.0f / 60.0f`**: The authoritative physics tick.
- **Accumulator**: Frame time (`dt` between renders) is added to an accumulator. The physics solver ticks as many times as `accumulator >= kFixedDt` allows. This ensures exactly identical simulation results whether the game renders at 30 FPS or 144 FPS.

## 2. Engine State & Storage (`GearEngine.hpp`)

All physics data lives in `EngineState`.
- **`RigidBodySoA`**: The canonical truth for physical properties. Fixed capacity (`kMaxBodies = 8192`). 
- **Why Fixed Capacity?** Dynamic arrays (`std::vector`) reallocate when their capacity is exceeded, meaning pointers shift in memory. A pre-allocated SoA guarantees absolute stability in memory addresses for the lifetime of the application, avoiding complex sparse-set mappings since our target body count is in the low thousands.

## 3. Constraint Solving (`ConstraintSolver.hpp`)

The solver is responsible for evaluating rules between bodies (e.g., locking two gears together).
- **Analytical Over Geometric**: Instead of relying on expensive mesh collisions to detect if two gear teeth are touching, the solver evaluates them analytically. It enforces the mathematical rule that if Body A rotates by X, Body B MUST rotate by `X * ratio`.
- **Iterative Solves**: The solver runs multiple iterations per `kFixedDt` (currently 8). In each iteration, it sequentially sweeps across all `GearConstraints` and `AxleConstraints`. This multi-pass approach allows the rigid bodies to globally converge on a stable state.

## 4. UI Architecture (`Theme.hpp`)

- **Dear ImGui**: We use immediate-mode UI for rendering the engineering workstation.
- **Encapsulation**: All `ImGuiStyle` modifications (colors, rounding parameters) are completely decoupled into `UI::ApplyTheme()`. 
- **Why?** Immediate mode UIs can notoriously clutter application logic. Isolating the "Agency-Quality" dark theme allows us to rapidly iterate on aesthetics without muddying the fixed timestep accumulator logic in `main.cpp`.
