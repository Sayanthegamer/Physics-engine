#pragma once

#include "GearEngine.hpp"
#include "ConstraintSolver.hpp"
#include <vector>
#include <variant>

namespace GearEngine {

// Deferred command queue structures
struct AddBodyCommand { 
    glm::vec3 position;
    float radius;
    // can add velocity/mass later
}; 
struct RemoveBodyCommand { EntityHandle handle; };
struct AddGearConstraintCommand { GearConstraint constraint; };
struct AddAxleConstraintCommand { AxleConstraint constraint; };

using CommandVariant = std::variant<
    AddBodyCommand,
    RemoveBodyCommand,
    AddGearConstraintCommand,
    AddAxleConstraintCommand
>;

class CommandQueue {
public:
    void PushCommand(const CommandVariant& cmd) {
        commands_.push_back(cmd);
    }

    void ApplyCommands(EngineState& state, ConstraintArrays& constraints) {
        for (const auto& cmd : commands_) {
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, AddBodyCommand>) {
                    EntityHandle handle = state.AllocateBody();
                    if (handle.IsValid()) {
                        RigidBodySoA& bodies = state.GetBodies();
                        uint32_t i = handle.index;
                        bodies.positions[i] = arg.position;
                        bodies.radii[i] = arg.radius;
                        bodies.linear_velocities[i] = glm::vec3(0.0f);
                        bodies.angular_velocities[i] = glm::vec3(0.0f);
                        bodies.rotations[i] = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
                        
                        // Physics Mass/Inertia Initialization
                        float mass = 3.14159f * arg.radius * arg.radius;
                        bodies.inverse_masses[i] = 1.0f / mass;
                        
                        float I_zz = 0.5f * mass * arg.radius * arg.radius;
                        bodies.inverse_inertias[i] = glm::mat3(0.0f);
                        bodies.inverse_inertias[i][2][2] = 1.0f / I_zz;
                    }
                }
                else if constexpr (std::is_same_v<T, RemoveBodyCommand>) {
                    state.RemoveBody(arg.handle);
                }
                else if constexpr (std::is_same_v<T, AddGearConstraintCommand>) {
                    constraints.gears.push_back(arg.constraint);
                }
                else if constexpr (std::is_same_v<T, AddAxleConstraintCommand>) {
                    constraints.axles.push_back(arg.constraint);
                }
            }, cmd);
        }
        commands_.clear();
    }

private:
    std::vector<CommandVariant> commands_;
};

} // namespace GearEngine
