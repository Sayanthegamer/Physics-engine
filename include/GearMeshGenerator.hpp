#ifndef GEARENGINE_INCLUDE_GEARMESHGENERATOR_H_
#define GEARENGINE_INCLUDE_GEARMESHGENERATOR_H_

#include <vector>
#include <glm/glm.hpp>

namespace gear_engine {

namespace involute {
    glm::vec2 CalculatePoint(float base_radius, float t);
}

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

class GearMeshGenerator {
public:
    GearMeshGenerator() = default;

    MeshData Generate(float pitch_radius, int num_teeth, float pressure_angle = 20.0f, float clearance = 0.1f);
};

} // namespace gear_engine

#endif // GEARENGINE_INCLUDE_GEARMESHGENERATOR_H_
