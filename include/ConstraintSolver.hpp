#pragma once

#include "GearEngine.hpp"
#include <vector>
#include <glm/glm.hpp>

namespace GearEngine {

// Analytical constraints storing just handles to the shared state
struct GearConstraint {
    EntityHandle body_a;
    EntityHandle body_b;
    float ratio;
    glm::vec3 axis_a;
    glm::vec3 axis_b;
    
    // Warm-starting variables (accumulated impulses)
    float accumulated_impulse = 0.0f;
};

struct AxleConstraint {
    EntityHandle body;
    glm::vec3 anchor_point;
    glm::vec3 axis;
    
    float accumulated_impulse = 0.0f;
};

// Flat contiguous arrays (No V-Tables)
struct ConstraintArrays {
    std::vector<GearConstraint> gears;
    std::vector<AxleConstraint> axles;
};

class ConstraintSolver {
public:
    ConstraintSolver() = default;

    void Solve(EngineState& state, ConstraintArrays& constraints, float dt, int iterations) {
        RigidBodySoA& bodies = state.GetBodies();

        // Iterative Sequential Impulse Loop
        for (int i = 0; i < iterations; ++i) {
            SolveGears(state, bodies, constraints.gears, dt);
            SolveAxles(state, bodies, constraints.axles, dt);
        }
    }

    void IntegratePositions(EngineState& state, float dt) {
        RigidBodySoA& bodies = state.GetBodies();
        uint32_t cap = state.GetCapacity();

        for (uint32_t i = 1; i < cap; ++i) {
            if (!state.IsIndexActive(i)) continue;

            bodies.positions[i] += bodies.linear_velocities[i] * dt;
            
            // Basic rotation integration (angular velocity vector -> quaternion)
            if (glm::length(bodies.angular_velocities[i]) > 0.0001f) {
                glm::quat spin(0.0f, bodies.angular_velocities[i].x, bodies.angular_velocities[i].y, bodies.angular_velocities[i].z);
                glm::quat new_rot = bodies.rotations[i] + (spin * bodies.rotations[i]) * (0.5f * dt);
                bodies.rotations[i] = glm::normalize(new_rot);
            }
        }
    }

private:
    void SolveGears(EngineState& state, RigidBodySoA& bodies, std::vector<GearConstraint>& gears, float /*dt*/) {
        for (auto& gear : gears) {
            if (!state.IsHandleValid(gear.body_a) || !state.IsHandleValid(gear.body_b)) {
                continue; // Stale handle detection
            }
            
            uint32_t a = gear.body_a.index;
            uint32_t b = gear.body_b.index;
            
            float invI_a = bodies.inverse_inertias[a][2][2];
            float invI_b = bodies.inverse_inertias[b][2][2];
            float ratio = gear.ratio;
            
            // 1D Jacobian: J = [1, ratio]
            // Effective mass Mc = (J * M^-1 * J^T)^-1
            float sum_inv_inertia = invI_a + (ratio * ratio * invI_b);
            if (sum_inv_inertia == 0.0f) continue;
            
            float mass_c = 1.0f / sum_inv_inertia;
            
            // Relative velocity (C_dot = J * V)
            float C_dot = bodies.angular_velocities[a].z + ratio * bodies.angular_velocities[b].z;
            
            // Impulse lambda
            float lambda = -mass_c * C_dot;
            
            // Apply impulse
            bodies.angular_velocities[a].z += invI_a * lambda;
            bodies.angular_velocities[b].z += ratio * invI_b * lambda;
        }
    }

    void SolveAxles(EngineState& state, RigidBodySoA& /*bodies*/, std::vector<AxleConstraint>& axles, float /*dt*/) {
        for (auto& axle : axles) {
            if (!state.IsHandleValid(axle.body)) {
                continue;
            }
            // Analytical axle solving logic (fixing translation or rotation axes)
        }
    }
};

} // namespace GearEngine
