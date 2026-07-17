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

## 5. Visual Debugging & Instanced Rendering (`DebugRenderer.hpp`)

To visualize the DOD state without compromising performance or adding heavy dependencies, we implemented a custom instanced OpenGL 3.3 renderer.
- **Custom GL Loader**: We bypass heavy dependencies (GLEW/GLAD) and avoid utilizing the limited ImGui internal loader by manually mapping only the exact 22 modern OpenGL functions we require (like `glDrawArraysInstanced`) via `glfwGetProcAddress`. This is highly robust and avoids header conflicts.
- **Instanced Uploads**: The renderer queries `IsIndexActive()` to pack only the *live* SoA data into a heap-allocated `std::vector` staging buffer. This ensures we never upload stale or dead handle data to the GPU.

## 6. Generational Memory & The Command Pipeline (`CommandQueue.hpp`)

- **Deferred Commands**: UI interactions (e.g., clicking "Add Gear") push payloads (`AddBodyCommand`) into a queue. These commands are executed sequentially *before* the physics loop begins.
- **Generational Free-List Allocator**: To prevent permanently exhausting the `kMaxBodies` limit when bodies are repeatedly created and destroyed, `EngineState` maintains a `free_indices_` stack. When a body is removed via `Clear All`, its index is pushed to the free stack and its `generation` integer increments. This instantly invalidates any stale `EntityHandle` pointing to that index across the entire codebase.

## 7. Integration Step Ordering (`ConstraintSolver.hpp`)

- **Sequential Impulse Flow**: The engine strict adheres to Box2D physics conventions. The loop order is explicitly:
  1. Evaluate Constraints & Solve Velocities (`solver.Solve()`)
  2. Integrate Velocities into Positions (`solver.IntegratePositions()`)
- **Why?** Updating positions *before* solving velocities leads to latent collision resolution and silent visual jitter when the constraint math is fully implemented.

## 8. Analytical Gear Constraints (Phase 3)

The core physics solver relies entirely on Jacobian-based velocity constraints rather than geometric collision detection.
- **1D Rotational Jacobian**: For performance and stability, gear interlocking is evaluated strictly along the Z-axis. The Jacobian matrix is implicitly $J = [1, ratio]$.
- **Effective Mass Calculation**: When a constraint is evaluated, we compute the effective mass as $M_c = (I_a^{-1} + ratio^2 \cdot I_b^{-1})^{-1}$. 
- **Sequential Impulse**: The solver calculates a corrective impulse $\lambda$ across 8 iterations per physics tick, instantly resolving conflicting angular velocities without the instability of spring-based penalty forces.

## 9. CAD-Style Interaction & Raycasting (Phase 4)

We eliminated the ImGui-based gear selection lists in favor of direct 3D spatial interaction.
- **Raycasting**: We utilize `glm::unProject` to cast a ray from the mouse's normalized device coordinates through the View/Projection matrix and intersect it with the Z=0 plane.
- **Auto-Snapping**: While hovering a preview gear, we iterate through the `RigidBodySoA` to find the nearest neighbor. If within tolerance, we calculate a tangent placement vector and automatically construct a `GearConstraint` bridging the two bodies upon placement.
- **Constraint Pruning**: The interaction model utilizes the `Delete` key on hovered gears. The `RemoveBodyCommand` was augmented to aggressively prune the `constraints.gears` array on execution, preventing dangling pointers or stale handles from crashing the solver.

## 10. SDF Gear Rendering & Teeth Quantization (Phase 5)

To visually validate the mathematical perfection of the Sequential Impulse solver, we implemented interlocking teeth.
- **Procedural SDF Generation**: Instead of CPU-side VBO generation (which scales poorly for $O(N)$ dynamic gears), we submit a 6-vertex quad array to the instanced pipeline. The Fragment Shader evaluates the pixel's polar coordinate $(r, \theta)$ and carves out teeth using a boundary derived from $1.0 + depth \cdot sign(\sin(Z \cdot \theta))$.
- **Discrete Gear Modules**: True mechanical gears must share a uniform tooth size (module). The engine tracks gears purely by their integer teeth count ($Z \in [4, 64]$). The physical radius is strictly derived via $R = Z / density$. This guarantees that a tiny 4-tooth gear and a massive 64-tooth gear mesh seamlessly without clipping.
