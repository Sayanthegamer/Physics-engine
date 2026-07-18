#include "GearMeshGenerator.hpp"
#include <cmath>

namespace gear_engine {

MeshData GearMeshGenerator::Generate(float radius, int num_teeth) {
    MeshData mesh;
    
    if (num_teeth <= 0 || radius <= 0.0f) {
        return mesh;
    }

    // We will build a gear mesh as a triangle fan for the front and back faces, 
    // and a triangle strip for the rim. For simplicity, we just generate raw triangles.
    
    // A gear has teeth. We'll divide the circle into (num_teeth * 4) segments.
    // Each tooth takes 2 segments (up, flat), and the gap takes 2 segments (down, flat).
    int segments = num_teeth * 4;
    float tooth_depth = radius * 0.2f; // Depth of the tooth
    float base_radius = radius - tooth_depth;
    float depth = 0.5f; // Gear thickness

    std::vector<glm::vec3> face_vertices;
    for (int i = 0; i < segments; ++i) {
        float angle = (float)i / segments * 2.0f * 3.14159265359f;
        
        // Determine radius based on whether we are at a tooth or gap
        int phase = i % 4; 
        float r = (phase == 1 || phase == 2) ? radius : base_radius;
        
        face_vertices.push_back(glm::vec3(r * cos(angle), r * sin(angle), depth));
    }

    // Generate front face (simple fan from center)
    glm::vec3 center_front(0.0f, 0.0f, depth);
    unsigned int center_idx = static_cast<unsigned int>(mesh.vertices.size());
    mesh.vertices.push_back({center_front, glm::vec3(0.0f, 0.0f, 1.0f)});
    
    for (int i = 0; i < segments; ++i) {
        mesh.vertices.push_back({face_vertices[i], glm::vec3(0.0f, 0.0f, 1.0f)});
    }
    
    for (int i = 0; i < segments; ++i) {
        mesh.indices.push_back(center_idx);
        mesh.indices.push_back(center_idx + 1 + i);
        mesh.indices.push_back(center_idx + 1 + ((i + 1) % segments));
    }
    
    return mesh;
}

} // namespace gear_engine
