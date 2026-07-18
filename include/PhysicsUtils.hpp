#ifndef GEARENGINE_INCLUDE_PHYSICSUTILS_H_
#define GEARENGINE_INCLUDE_PHYSICSUTILS_H_

#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

#include "GearEngine.hpp"

namespace gear_engine {
namespace physics_utils {

    // Calculates the required rotation (phi_b) for a newly placed gear B to perfectly mesh with gear A.
    // Assumes gears have teeth equal to round(radius * 4.0) and teeth are centered at 0 phase.
    inline float CalculateMeshRotationOffset(float r_a, float r_b, glm::vec3 pos_a, glm::vec3 pos_b, float phi_a) {
        float n_a = std::max(4.0f, std::round(r_a * 4.0f));
        float n_b = std::max(4.0f, std::round(r_b * 4.0f));
        
        float theta_ab = std::atan2(pos_b.y - pos_a.y, pos_b.x - pos_a.x);
        float pi = 3.14159265359f;
        
        float phi_b = theta_ab + pi - (pi / n_b) + (n_a / n_b) * (theta_ab - phi_a);
        return phi_b;
    }
    
    // Calculates the kinematic transmission ratio between two gears (omega_out / omega_in)
    // Returns the multiplier for the output gear's speed.
    inline float CalculateTransmissionRatio(GearType type_in, GearType type_out, float r_in, float r_out) {
        // Internal gears mesh with their internal teeth, rotating in the SAME direction as the internal gear.
        if (type_in == GearType::Internal || type_out == GearType::Internal) {
            return r_in / r_out;
        }
        
        // Rack to Pinion (Linear to Rotational) -> omega = v / r_out
        if (type_in == GearType::Rack && type_out != GearType::Rack) {
            return 1.0f / r_out; 
        }
        
        // Pinion to Rack (Rotational to Linear) -> v = omega * r_in
        if (type_out == GearType::Rack && type_in != GearType::Rack) {
            return r_in;
        }
        
        // Basic ratio based on pitch radii
        float ratio = r_in / r_out;
        
        // Spur, Helical, Bevel, and Worm gears inherently reverse the direction of rotation (relative to the meshing axis)
        if (type_in == GearType::Spur || type_in == GearType::Helical || type_in == GearType::Bevel || type_in == GearType::Worm) {
            return -ratio;
        }
        
        return -ratio; // Default fallback
    }

    // Determines if two gears can physically mesh based on their type and parameters (e.g. Helical angles)
    inline bool AreGearsCompatible(GearType type_a, float param0_a, GearType type_b, float param0_b) {
        if (type_a == GearType::Helical || type_b == GearType::Helical) {
            if (type_a != type_b) return false; // Helical only meshes with Helical
            
            // For parallel helical gears, angles must be equal and opposite
            float sum = param0_a + param0_b;
            if (std::abs(sum) > 0.01f) {
                return false;
            }
            return true;
        }
        
        if (type_a == GearType::Bevel || type_b == GearType::Bevel) {
            if (type_a != type_b) return false; // Bevels only mesh with bevels
            return true;
        }
        
        if (type_a == GearType::Worm || type_b == GearType::Worm) {
            if (type_a == GearType::Worm && type_b == GearType::Spur) return true;
            if (type_a == GearType::Spur && type_b == GearType::Worm) return true;
            return false;
        }
        
        return true;
    }

} // namespace physics_utils
} // namespace gear_engine

#endif // GEARENGINE_INCLUDE_PHYSICSUTILS_H_
