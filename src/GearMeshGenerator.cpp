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
    float tooth_depth = 0.18f; // Constant depth slightly reduced to prevent corner clipping
    float outer_radius = radius + tooth_depth * 0.5f;
    float root_radius  = radius - tooth_depth * 0.5f;
    float depth = 0.5f; // Gear thickness

    std::vector<glm::vec3> face_vertices_front;
    std::vector<glm::vec3> face_vertices_back;
    for (int i = 0; i < segments; ++i) {
        float angle = (float)i / segments * 2.0f * 3.14159265359f;
        
        // Determine radius based on whether we are at a tooth or gap
        int phase = i % 4; 
        float r = (phase == 1 || phase == 2) ? outer_radius : root_radius;
        
        face_vertices_front.push_back(glm::vec3(r * cos(angle), r * sin(angle), depth));
        face_vertices_back.push_back(glm::vec3(r * cos(angle), r * sin(angle), -depth));
    }

    // 1. Generate front face (simple fan from center)
    glm::vec3 center_front(0.0f, 0.0f, depth);
    unsigned int center_front_idx = static_cast<unsigned int>(mesh.vertices.size());
    mesh.vertices.push_back({center_front, glm::vec3(0.0f, 0.0f, 1.0f)});
    
    for (int i = 0; i < segments; ++i) {
        mesh.vertices.push_back({face_vertices_front[i], glm::vec3(0.0f, 0.0f, 1.0f)});
    }
    
    for (int i = 0; i < segments; ++i) {
        mesh.indices.push_back(center_front_idx);
        mesh.indices.push_back(center_front_idx + 1 + i);
        mesh.indices.push_back(center_front_idx + 1 + ((i + 1) % segments));
    }
    
    // 2. Generate back face (winding reversed to face outward)
    glm::vec3 center_back(0.0f, 0.0f, -depth);
    unsigned int center_back_idx = static_cast<unsigned int>(mesh.vertices.size());
    mesh.vertices.push_back({center_back, glm::vec3(0.0f, 0.0f, -1.0f)});
    
    for (int i = 0; i < segments; ++i) {
        mesh.vertices.push_back({face_vertices_back[i], glm::vec3(0.0f, 0.0f, -1.0f)});
    }
    
    for (int i = 0; i < segments; ++i) {
        mesh.indices.push_back(center_back_idx);
        mesh.indices.push_back(center_back_idx + 1 + ((i + 1) % segments));
        mesh.indices.push_back(center_back_idx + 1 + i);
    }
    
    // 3. Generate rim (sides) with flat shading normals
    for (int i = 0; i < segments; ++i) {
        int next_i = (i + 1) % segments;
        glm::vec3 v0 = face_vertices_front[i];
        glm::vec3 v1 = face_vertices_back[i];
        glm::vec3 v2 = face_vertices_front[next_i];
        glm::vec3 v3 = face_vertices_back[next_i];
        
        glm::vec3 edge1 = v2 - v0;
        glm::vec3 edge2 = v1 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        
        unsigned int start_idx = static_cast<unsigned int>(mesh.vertices.size());
        mesh.vertices.push_back({v0, normal});
        mesh.vertices.push_back({v1, normal});
        mesh.vertices.push_back({v2, normal});
        mesh.vertices.push_back({v3, normal});
        
        // Triangle 1: v0, v1, v2
        mesh.indices.push_back(start_idx);
        mesh.indices.push_back(start_idx + 1);
        mesh.indices.push_back(start_idx + 2);
        
        // Triangle 2: v1, v3, v2
        mesh.indices.push_back(start_idx + 1);
        mesh.indices.push_back(start_idx + 3);
        mesh.indices.push_back(start_idx + 2);
    }
    
    return mesh;
}

} // namespace gear_engine
