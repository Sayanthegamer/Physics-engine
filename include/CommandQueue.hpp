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
                        bodies.positions[handle.index] = arg.position;
                        bodies.radii[handle.index] = arg.radius;
                        // Zero out velocities for now
                        bodies.linear_velocities[handle.index] = glm::vec3(0.0f);
                        bodies.angular_velocities[handle.index] = glm::vec3(0.0f);
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
