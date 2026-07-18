#ifndef GEARENGINE_INCLUDE_GEARMESHGENERATOR_H_
#define GEARENGINE_INCLUDE_GEARMESHGENERATOR_H_

#include <vector>
#include <glm/glm.hpp>

namespace gear_engine {

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

    MeshData Generate(float radius, int num_teeth);
};

} // namespace gear_engine

#endif // GEARENGINE_INCLUDE_GEARMESHGENERATOR_H_
