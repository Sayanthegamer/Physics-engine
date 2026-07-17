#pragma once

#include "GearEngine.hpp"
#include "ConstraintSolver.hpp"
#include <vector>
#include <variant>

namespace GearEngine {

// Deferred command queue structures
struct AddBodyCommand {}; 
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
                    state.AllocateBody();
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
