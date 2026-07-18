#pragma once

#include "GearEngine.hpp"
#include <vector>
#include <glm/glm.hpp>

namespace gear_engine {

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

struct MotorConstraint {
    EntityHandle body;
    float target_speed;
    
    float accumulated_impulse = 0.0f;
};

struct LinearGearConstraint {
    EntityHandle body_rotational;
    EntityHandle body_linear;
    float ratio; // Radius of pinion
    glm::vec3 axis_rotational;
    glm::vec3 axis_linear;
    
    float accumulated_impulse = 0.0f;
};

struct ConstraintArrays {
    std::vector<GearConstraint> gears;
    std::vector<LinearGearConstraint> linear_gears;
    std::vector<AxleConstraint> axles;
    std::vector<MotorConstraint> motors;
};

class ConstraintSolver {
public:
    ConstraintSolver() = default;

    void CheckAutoDisconnect(EngineState& state, ConstraintArrays& constraints, float threshold) {
        auto& bodies = state.GetBodies();
        
        // Remove gears that are too far apart
        constraints.gears.erase(
            std::remove_if(constraints.gears.begin(), constraints.gears.end(),
                [&](const GearConstraint& gc) {
                    if (!state.IsHandleValid(gc.body_a) || !state.IsHandleValid(gc.body_b)) {
                        return true; // Remove stale constraints
                    }
                    uint32_t a = gc.body_a.index;
                    uint32_t b = gc.body_b.index;
                    
                    float dist = glm::distance(bodies.positions[a], bodies.positions[b]);
                    float expected_dist = bodies.radii[a] + bodies.radii[b];
                    
                    return dist > (expected_dist + threshold);
                }),
            constraints.gears.end()
        );
    }

    void Solve(EngineState& state, ConstraintArrays& constraints, float dt, int iterations) {
        RigidBodySoA& bodies = state.GetBodies();

        // Iterative Sequential Impulse Loop
        for (int i = 0; i < iterations; ++i) {
            SolveGears(state, bodies, constraints.gears, dt);
            SolveLinearGears(state, bodies, constraints.linear_gears, dt);
            SolveAxles(state, bodies, constraints.axles, dt);
            SolveMotors(state, bodies, constraints.motors, dt);
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
            
            glm::mat3 invI_a = bodies.inverse_inertias[a];
            glm::mat3 invI_b = bodies.inverse_inertias[b];
            float ratio = gear.ratio;
            glm::vec3 axis_a = gear.axis_a;
            glm::vec3 axis_b = gear.axis_b;
            
            // 3D Jacobian: J_a = axis_a, J_b = ratio * axis_b
            float inv_inertia_a = glm::dot(axis_a, invI_a * axis_a);
            float inv_inertia_b = glm::dot(axis_b, invI_b * axis_b);
            
            // Worm gear non-backdrivability (Perfect lock)
            if (bodies.gear_types[b] == GearType::Worm && bodies.gear_types[a] != GearType::Worm) {
                inv_inertia_b = 0.0f; // Worm cannot be driven, acts as infinite inertia
                invI_b = glm::mat3(0.0f);
            } else if (bodies.gear_types[a] == GearType::Worm && bodies.gear_types[b] != GearType::Worm) {
                inv_inertia_a = 0.0f; 
                invI_a = glm::mat3(0.0f);
            }
            
            float sum_inv_inertia = inv_inertia_a + (ratio * ratio * inv_inertia_b);
            if (sum_inv_inertia == 0.0f) continue;
            
            float mass_c = 1.0f / sum_inv_inertia;
            
            // Relative velocity (C_dot = J * V)
            float C_dot = glm::dot(bodies.angular_velocities[a], axis_a) + 
                          ratio * glm::dot(bodies.angular_velocities[b], axis_b);
            
            // Impulse lambda
            float lambda = -mass_c * C_dot;
            
            // Apply impulse
            bodies.angular_velocities[a] += (invI_a * axis_a) * lambda;
            bodies.angular_velocities[b] += (invI_b * axis_b) * (ratio * lambda);
        }
    }

    void SolveLinearGears(EngineState& state, RigidBodySoA& bodies, std::vector<LinearGearConstraint>& linear_gears, float /*dt*/) {
        for (auto& gear : linear_gears) {
            if (!state.IsHandleValid(gear.body_rotational) || !state.IsHandleValid(gear.body_linear)) {
                continue;
            }
            
            uint32_t rot = gear.body_rotational.index;
            uint32_t lin = gear.body_linear.index;
            
            glm::mat3 invI_rot = bodies.inverse_inertias[rot];
            float invM_lin = bodies.inverse_masses[lin];
            float ratio = gear.ratio;
            glm::vec3 axis_rot = gear.axis_rotational;
            glm::vec3 axis_lin = gear.axis_linear;
            
            // 3D Jacobian: J_rot = ratio * axis_rot, J_lin = axis_lin
            float inv_inertia_rot = glm::dot(axis_rot, invI_rot * axis_rot);
            float inv_inertia_lin = invM_lin * glm::dot(axis_lin, axis_lin);
            
            float sum_inv_inertia = (ratio * ratio * inv_inertia_rot) + inv_inertia_lin;
            if (sum_inv_inertia == 0.0f) continue;
            
            float mass_c = 1.0f / sum_inv_inertia;
            
            // Relative velocity
            float C_dot = ratio * glm::dot(bodies.angular_velocities[rot], axis_rot) + 
                          glm::dot(bodies.linear_velocities[lin], axis_lin);
            
            float lambda = -mass_c * C_dot;
            
            // Apply impulse
            bodies.angular_velocities[rot] += (invI_rot * axis_rot) * (ratio * lambda);
            bodies.linear_velocities[lin] += axis_lin * (invM_lin * lambda);
        }
    }

    void SolveAxles(EngineState& state, [[maybe_unused]] RigidBodySoA& bodies, std::vector<AxleConstraint>& axles, float /*dt*/) {
        for (auto& axle : axles) {
            if (!state.IsHandleValid(axle.body)) {
                continue;
            }
            // Analytical axle solving logic (fixing translation or rotation axes)
        }
    }

    void SolveMotors(EngineState& state, RigidBodySoA& bodies, std::vector<MotorConstraint>& motors, float /*dt*/) {
        for (auto& motor : motors) {
            if (!state.IsHandleValid(motor.body)) continue;
            
            uint32_t a = motor.body.index;
            float invI = bodies.inverse_inertias[a][2][2];
            if (invI == 0.0f) continue;
            
            float mass_c = 1.0f / invI;
            float C_dot = bodies.angular_velocities[a].z - motor.target_speed;
            float lambda = -mass_c * C_dot;
            
            bodies.angular_velocities[a].z += invI * lambda;
        }
    }
};

} // namespace gear_engine
