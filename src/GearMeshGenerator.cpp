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

    if (gear_type == GearType::Rack) {
        float length = param0 > 0.1f ? param0 : 10.0f;
        float tooth_pitch = (2.0f * 3.14159265359f * pitch_radius) / num_teeth;
        float addendum = tooth_pitch / 3.14159265359f;
        float dedendum = addendum + clearance;
        float depth = 0.5f;

        std::vector<glm::vec2> profile;
        float start_x = -length / 2.0f;
        int num_rack_teeth = (int)(length / tooth_pitch);
        
        // Base of the rack
        profile.push_back(glm::vec2(start_x, -pitch_radius - 2.0f * dedendum));
        
        for(int i = 0; i <= num_rack_teeth; ++i) {
            float x = start_x + i * tooth_pitch;
            float px = tooth_pitch / 4.0f;
            float pa_tan = std::tan(glm::radians(pressure_angle));
            
            // Trapezoidal teeth
            profile.push_back(glm::vec2(x - px - dedendum*pa_tan, -pitch_radius - dedendum));
            profile.push_back(glm::vec2(x - px + addendum*pa_tan, -pitch_radius + addendum));
            profile.push_back(glm::vec2(x + px - addendum*pa_tan, -pitch_radius + addendum));
            profile.push_back(glm::vec2(x + px + dedendum*pa_tan, -pitch_radius - dedendum));
        }
        profile.push_back(glm::vec2(start_x + (num_rack_teeth+1) * tooth_pitch, -pitch_radius - 2.0f * dedendum));

        // Front face
        unsigned int start_front = static_cast<unsigned int>(mesh.vertices.size());
        for (const auto& p : profile) mesh.vertices.push_back({glm::vec3(p.x, p.y, depth), glm::vec3(0,0,1)});
        // Simple fan from a point far below (not perfect but OK for rack base)
        glm::vec3 rack_center(start_x + length/2.0f, -pitch_radius - 4.0f * dedendum, depth);
        mesh.vertices.push_back({rack_center, glm::vec3(0,0,1)});
        unsigned int center_idx = static_cast<unsigned int>(mesh.vertices.size()) - 1;
        for (unsigned int i = 0; i < profile.size() - 1; ++i) {
            mesh.indices.push_back(center_idx);
            mesh.indices.push_back(start_front + i);
            mesh.indices.push_back(start_front + i + 1);
        }

        // Back face
        unsigned int start_back = static_cast<unsigned int>(mesh.vertices.size());
        for (const auto& p : profile) mesh.vertices.push_back({glm::vec3(p.x, p.y, -depth), glm::vec3(0,0,-1)});
        glm::vec3 rack_center_b(start_x + length/2.0f, -pitch_radius - 4.0f * dedendum, -depth);
        mesh.vertices.push_back({rack_center_b, glm::vec3(0,0,-1)});
        unsigned int center_idx_b = static_cast<unsigned int>(mesh.vertices.size()) - 1;
        for (unsigned int i = 0; i < profile.size() - 1; ++i) {
            mesh.indices.push_back(center_idx_b);
            mesh.indices.push_back(start_back + i + 1);
            mesh.indices.push_back(start_back + i);
        }

        // Rim
        for (unsigned int i = 0; i < profile.size() - 1; ++i) {
            glm::vec3 v0(profile[i].x, profile[i].y, depth);
            glm::vec3 v1(profile[i].x, profile[i].y, -depth);
            glm::vec3 v2(profile[i+1].x, profile[i+1].y, depth);
            glm::vec3 v3(profile[i+1].x, profile[i+1].y, -depth);
            
            glm::vec3 n = glm::normalize(glm::cross(v2 - v0, v1 - v0));
            unsigned int start = static_cast<unsigned int>(mesh.vertices.size());
            mesh.vertices.push_back({v0, n}); mesh.vertices.push_back({v1, n});
            mesh.vertices.push_back({v2, n}); mesh.vertices.push_back({v3, n});
            
            mesh.indices.push_back(start); mesh.indices.push_back(start+1); mesh.indices.push_back(start+2);
            mesh.indices.push_back(start+1); mesh.indices.push_back(start+3); mesh.indices.push_back(start+2);
        }
        return mesh;
    }

    if (gear_type == GearType::Internal) {
        // Simplistic internal gear (ring with teeth)
        float module = (2.0f * pitch_radius) / num_teeth;
        float addendum = module;
        float dedendum = module + clearance;
        // float inner_radius = pitch_radius - addendum;
        float outer_radius = pitch_radius + dedendum + 0.5f; // Ring thickness
        float depth = 0.5f;

        // Generate inner profile (simplified to sine wave for visual fidelity without complex CSG)
        int segments = num_teeth * 10;
        std::vector<glm::vec2> inner_profile;
        std::vector<glm::vec2> outer_profile;
        for (int i = 0; i < segments; ++i) {
            float angle = (i / (float)segments) * 2.0f * 3.14159265359f;
            float r = pitch_radius + addendum * std::sin(angle * num_teeth);
            inner_profile.push_back(glm::vec2(r * std::cos(angle), r * std::sin(angle)));
            outer_profile.push_back(glm::vec2(outer_radius * std::cos(angle), outer_radius * std::sin(angle)));
        }

        auto AddRingFace = [&](float z, glm::vec3 n, bool reverse) {
            unsigned int start = static_cast<unsigned int>(mesh.vertices.size());
            for (int i = 0; i < segments; ++i) {
                mesh.vertices.push_back({glm::vec3(inner_profile[i].x, inner_profile[i].y, z), n});
                mesh.vertices.push_back({glm::vec3(outer_profile[i].x, outer_profile[i].y, z), n});
            }
            for (int i = 0; i < segments; ++i) {
                int next_i = (i + 1) % segments;
                unsigned int i0 = start + i * 2;
                unsigned int o0 = start + i * 2 + 1;
                unsigned int i1 = start + next_i * 2;
                unsigned int o1 = start + next_i * 2 + 1;
                
                if (!reverse) {
                    mesh.indices.push_back(i0); mesh.indices.push_back(o0); mesh.indices.push_back(i1);
                    mesh.indices.push_back(o0); mesh.indices.push_back(o1); mesh.indices.push_back(i1);
                } else {
                    mesh.indices.push_back(i0); mesh.indices.push_back(i1); mesh.indices.push_back(o0);
                    mesh.indices.push_back(o0); mesh.indices.push_back(i1); mesh.indices.push_back(o1);
                }
            }
        };

        AddRingFace(depth, glm::vec3(0,0,1), false);
        AddRingFace(-depth, glm::vec3(0,0,-1), true);

        // Inner rim
        for (int i = 0; i < segments; ++i) {
            int next_i = (i + 1) % segments;
            glm::vec3 v0(inner_profile[i].x, inner_profile[i].y, depth);
            glm::vec3 v1(inner_profile[i].x, inner_profile[i].y, -depth);
            glm::vec3 v2(inner_profile[next_i].x, inner_profile[next_i].y, depth);
            glm::vec3 v3(inner_profile[next_i].x, inner_profile[next_i].y, -depth);
            
            glm::vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            unsigned int start = static_cast<unsigned int>(mesh.vertices.size());
            mesh.vertices.push_back({v0, n}); mesh.vertices.push_back({v1, n});
            mesh.vertices.push_back({v2, n}); mesh.vertices.push_back({v3, n});
            mesh.indices.push_back(start); mesh.indices.push_back(start+1); mesh.indices.push_back(start+2);
            mesh.indices.push_back(start+1); mesh.indices.push_back(start+3); mesh.indices.push_back(start+2);
        }
        
        // Outer rim
        for (int i = 0; i < segments; ++i) {
            int next_i = (i + 1) % segments;
            glm::vec3 v0(outer_profile[i].x, outer_profile[i].y, depth);
            glm::vec3 v1(outer_profile[i].x, outer_profile[i].y, -depth);
            glm::vec3 v2(outer_profile[next_i].x, outer_profile[next_i].y, depth);
            glm::vec3 v3(outer_profile[next_i].x, outer_profile[next_i].y, -depth);
            
            glm::vec3 n = glm::normalize(glm::cross(v2 - v0, v1 - v0));
            unsigned int start = static_cast<unsigned int>(mesh.vertices.size());
            mesh.vertices.push_back({v0, n}); mesh.vertices.push_back({v1, n});
            mesh.vertices.push_back({v2, n}); mesh.vertices.push_back({v3, n});
            mesh.indices.push_back(start); mesh.indices.push_back(start+1); mesh.indices.push_back(start+2);
            mesh.indices.push_back(start+1); mesh.indices.push_back(start+3); mesh.indices.push_back(start+2);
        }
        return mesh;
    }

    // Basic gear math (for Spur, Helical, Bevel, Worm)
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
        if (gear_type == GearType::Helical || gear_type == GearType::Worm) {
            float twist = param0 * 3.14159265359f / 180.0f;
            if (gear_type == GearType::Worm) twist = (90.0f - param0) * 3.14159265359f / 180.0f; // Lead angle to helix angle
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
