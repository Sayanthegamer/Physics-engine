#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace gear_engine {

// Use compile-time constant as requested
constexpr uint32_t kMaxBodies = 8192;

struct EntityHandle {
    uint32_t index = 0;
    uint32_t generation = 0;

    bool IsValid() const { return index != 0; } // Assuming 0 is invalid/unallocated
};

enum class GearType : uint8_t {
    Spur = 0,
    Helical = 1,
    Bevel = 2,
    Worm = 3,
    Rack = 4,
    Internal = 5
};

// Data-Oriented shared state using pure SoA layout
struct RigidBodySoA {
    glm::vec3 positions[kMaxBodies];
    glm::quat rotations[kMaxBodies];
    glm::vec3 linear_velocities[kMaxBodies];
    glm::vec3 angular_velocities[kMaxBodies];
    float inverse_masses[kMaxBodies];
    glm::mat3 inverse_inertias[kMaxBodies];
    float radii[kMaxBodies]; // Added for visual debugging and collision
    float pressure_angles[kMaxBodies]; // Involute profile parameter
    float clearances[kMaxBodies];      // Involute profile parameter
    
    // New parameters for diverse gear types
    GearType gear_types[kMaxBodies];
    float gear_params_0[kMaxBodies];
    float gear_params_1[kMaxBodies];
};

class EngineState {
public:
    EngineState() = default;

    EntityHandle AllocateBody() {
        uint32_t index = 0;
        if (!free_indices_.empty()) {
            index = free_indices_.back();
            free_indices_.pop_back();
        } else {
            if (next_free_index_ >= kMaxBodies) {
                return {0, 0}; // Reached maximum capacity
            }
            index = next_free_index_++;
        }
        is_active_[index] = true;
        return {index, generations_[index]};
    }

    void RemoveBody(EntityHandle handle) {
        if (IsHandleValid(handle)) {
            generations_[handle.index]++;
            is_active_[handle.index] = false;
            free_indices_.push_back(handle.index);
        }
    }

    bool IsHandleValid(EntityHandle handle) const {
        if (handle.index == 0 || handle.index >= kMaxBodies) return false;
        return generations_[handle.index] == handle.generation && is_active_[handle.index];
    }
    
    bool IsIndexActive(uint32_t index) const {
        if (index == 0 || index >= kMaxBodies) return false;
        return is_active_[index];
    }

    uint32_t GetGeneration(uint32_t index) const {
        return generations_[index];
    }

    uint32_t GetCapacity() const { return next_free_index_; }

    RigidBodySoA& GetBodies() { return bodies_; }
    const RigidBodySoA& GetBodies() const { return bodies_; }

private:
    RigidBodySoA bodies_;
    uint32_t generations_[kMaxBodies] = {0};
    bool is_active_[kMaxBodies] = {false};
    std::vector<uint32_t> free_indices_;
    uint32_t next_free_index_ = 1; // 0 reserved as invalid handle
};

} // namespace gear_engine
