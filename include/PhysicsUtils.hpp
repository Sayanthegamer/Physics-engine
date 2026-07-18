#ifndef GEARENGINE_INCLUDE_PHYSICSUTILS_H_
#define GEARENGINE_INCLUDE_PHYSICSUTILS_H_

#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

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

} // namespace physics_utils
} // namespace gear_engine

#endif // GEARENGINE_INCLUDE_PHYSICSUTILS_H_
