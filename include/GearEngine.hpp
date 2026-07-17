#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace GearEngine {

// Use compile-time constant as requested
constexpr uint32_t kMaxBodies = 8192;

struct EntityHandle {
    uint32_t index = 0;
    uint32_t generation = 0;

    bool IsValid() const { return index != 0; } // Assuming 0 is invalid/unallocated
};

// Data-Oriented shared state using pure SoA layout
struct RigidBodySoA {
    glm::vec3 positions[kMaxBodies];
    glm::quat rotations[kMaxBodies];
    glm::vec3 linear_velocities[kMaxBodies];
    glm::vec3 angular_velocities[kMaxBodies];
    float inverse_masses[kMaxBodies];
    glm::mat3 inverse_inertias[kMaxBodies];
};

class EngineState {
public:
    EngineState() = default;

    EntityHandle AllocateBody() {
        if (next_free_index_ >= kMaxBodies) {
            // Reached maximum capacity
            return {0, 0};
        }
        uint32_t index = next_free_index_++;
        return {index, generations_[index]};
    }

    void RemoveBody(EntityHandle handle) {
        if (IsHandleValid(handle)) {
            // Increment generation to instantly invalidate all stale indices pointing here
            generations_[handle.index]++;
            // (A robust system would also add handle.index to a free-list queue here)
        }
    }

    bool IsHandleValid(EntityHandle handle) const {
        if (handle.index == 0 || handle.index >= kMaxBodies) return false;
        return generations_[handle.index] == handle.generation;
    }

    RigidBodySoA& GetBodies() { return bodies_; }
    const RigidBodySoA& GetBodies() const { return bodies_; }

private:
    RigidBodySoA bodies_;
    uint32_t generations_[kMaxBodies] = {0};
    uint32_t next_free_index_ = 1; // 0 reserved as invalid handle
};

} // namespace GearEngine
