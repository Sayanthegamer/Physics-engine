#include "GearMeshGenerator.hpp"
#include <cmath>

namespace gear_engine {

namespace involute {
    glm::vec2 CalculatePoint(float base_radius, float t) {
        return glm::vec2(
            base_radius * (std::cos(t) + t * std::sin(t)),
            base_radius * (std::sin(t) - t * std::cos(t))
        );
    }
}

MeshData GearMeshGenerator::Generate(float pitch_radius, int num_teeth, float pressure_angle, float clearance, GearType gear_type, float param0) {
    MeshData mesh;
    
    if (num_teeth <= 0 || pitch_radius <= 0.0f) {
        return mesh;
    }

    // Basic gear math
    float module = (2.0f * pitch_radius) / num_teeth;
    float addendum = module;
    float dedendum = module + clearance;
    float outer_radius = pitch_radius + addendum;
    float root_radius  = pitch_radius - dedendum;
    float base_radius  = pitch_radius * std::cos(glm::radians(pressure_angle));
    
    float depth = 0.5f;

    // We generate the gear outline as a series of 2D points, then extrude it.
    std::vector<glm::vec2> profile_points;
    
    float pitch_angle = (2.0f * 3.14159265359f) / num_teeth;
    float alpha = glm::radians(pressure_angle);
    float inv_alpha = std::tan(alpha) - alpha;
    
    float tooth_thickness_angle = pitch_angle / 2.0f;
    float base_angle_offset = (tooth_thickness_angle / 2.0f) + inv_alpha;
    
    // Max t parameter where involute reaches outer_radius
    float t_max = 0.0f;
    if (outer_radius > base_radius) {
        t_max = std::sqrt((outer_radius / base_radius) * (outer_radius / base_radius) - 1.0f);
    }
    
    int involute_segments = 20; 

    for (int i = 0; i < num_teeth; ++i) {
        float center_angle = i * pitch_angle;
        
        // 1. Root circle point (start of tooth)
        float start_angle = center_angle - pitch_angle / 2.0f;
        profile_points.push_back(glm::vec2(root_radius * std::cos(start_angle), root_radius * std::sin(start_angle)));
        
        // 2. Involute curve (up)
        for(int j = 0; j <= involute_segments; ++j) {
            float t = (float)j / involute_segments * t_max;
            glm::vec2 p = involute::CalculatePoint(base_radius, t);
            float rot = center_angle - base_angle_offset;
            float px = p.x * std::cos(rot) - p.y * std::sin(rot);
            float py = p.x * std::sin(rot) + p.y * std::cos(rot);
            
            float r = std::sqrt(px*px + py*py);
            if(r >= root_radius) {
                profile_points.push_back(glm::vec2(px, py));
            } else if (j == 0) {
                profile_points.push_back(glm::vec2(root_radius * std::cos(rot), root_radius * std::sin(rot)));
            }
        }
        
        // 3. Involute curve (down)
        for(int j = involute_segments; j >= 0; --j) {
            float t = (float)j / involute_segments * t_max;
            glm::vec2 p = involute::CalculatePoint(base_radius, t);
            float rot = center_angle + base_angle_offset;
            float px = p.x * std::cos(rot) - (-p.y) * std::sin(rot);
            float py = p.x * std::sin(rot) + (-p.y) * std::cos(rot);
            
            float r = std::sqrt(px*px + py*py);
            if(r >= root_radius) {
                profile_points.push_back(glm::vec2(px, py));
            } else if (j == 0) {
                profile_points.push_back(glm::vec2(root_radius * std::cos(rot), root_radius * std::sin(rot)));
            }
        }
        
        // 4. Root circle point (end of tooth)
        float end_angle = center_angle + pitch_angle / 2.0f;
        profile_points.push_back(glm::vec2(root_radius * std::cos(end_angle), root_radius * std::sin(end_angle)));
    }

    auto TransformPoint = [&](const glm::vec2& p, float z) -> glm::vec2 {
        glm::vec2 res = p;
        if (gear_type == GearType::Helical) {
            float twist = param0 * 3.14159265359f / 180.0f;
            float angle = (z / pitch_radius) * std::tan(twist);
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);
            res = glm::vec2(p.x * cosA - p.y * sinA, p.x * sinA + p.y * cosA);
        } else if (gear_type == GearType::Bevel) {
            float cone_angle = param0 * 3.14159265359f / 180.0f;
            float scale = 1.0f - (z / pitch_radius) * std::tan(cone_angle);
            if (scale < 0.1f) scale = 0.1f;
            res = p * scale;
        }
        return res;
    };

    int points_count = (int)profile_points.size();

    // 1. Front face
    glm::vec3 center_front(0.0f, 0.0f, depth);
    unsigned int center_front_idx = static_cast<unsigned int>(mesh.vertices.size());
    mesh.vertices.push_back({center_front, glm::vec3(0.0f, 0.0f, 1.0f)});
    for (int i = 0; i < points_count; ++i) {
        glm::vec2 tf = TransformPoint(profile_points[i], depth);
        mesh.vertices.push_back({glm::vec3(tf.x, tf.y, depth), glm::vec3(0.0f, 0.0f, 1.0f)});
    }
    for (int i = 0; i < points_count; ++i) {
        mesh.indices.push_back(center_front_idx);
        mesh.indices.push_back(center_front_idx + 1 + i);
        mesh.indices.push_back(center_front_idx + 1 + ((i + 1) % points_count));
    }

    // 2. Back face
    glm::vec3 center_back(0.0f, 0.0f, -depth);
    unsigned int center_back_idx = static_cast<unsigned int>(mesh.vertices.size());
    mesh.vertices.push_back({center_back, glm::vec3(0.0f, 0.0f, -1.0f)});
    for (int i = 0; i < points_count; ++i) {
        glm::vec2 tb = TransformPoint(profile_points[i], -depth);
        mesh.vertices.push_back({glm::vec3(tb.x, tb.y, -depth), glm::vec3(0.0f, 0.0f, -1.0f)});
    }
    for (int i = 0; i < points_count; ++i) {
        mesh.indices.push_back(center_back_idx);
        mesh.indices.push_back(center_back_idx + 1 + ((i + 1) % points_count));
        mesh.indices.push_back(center_back_idx + 1 + i);
    }

    // 3. Rim
    for (int i = 0; i < points_count; ++i) {
        int next_i = (i + 1) % points_count;
        glm::vec2 tf0 = TransformPoint(profile_points[i], depth);
        glm::vec2 tb0 = TransformPoint(profile_points[i], -depth);
        glm::vec2 tf1 = TransformPoint(profile_points[next_i], depth);
        glm::vec2 tb1 = TransformPoint(profile_points[next_i], -depth);

        glm::vec3 v0(tf0.x, tf0.y, depth);
        glm::vec3 v1(tb0.x, tb0.y, -depth);
        glm::vec3 v2(tf1.x, tf1.y, depth);
        glm::vec3 v3(tb1.x, tb1.y, -depth);
        
        glm::vec3 edge1 = v2 - v0;
        glm::vec3 edge2 = v1 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        
        unsigned int start_idx = static_cast<unsigned int>(mesh.vertices.size());
        mesh.vertices.push_back({v0, normal});
        mesh.vertices.push_back({v1, normal});
        mesh.vertices.push_back({v2, normal});
        mesh.vertices.push_back({v3, normal});
        
        mesh.indices.push_back(start_idx);
        mesh.indices.push_back(start_idx + 1);
        mesh.indices.push_back(start_idx + 2);
        
        mesh.indices.push_back(start_idx + 1);
        mesh.indices.push_back(start_idx + 3);
        mesh.indices.push_back(start_idx + 2);
    }
    
    return mesh;

}

} // namespace gear_engine
