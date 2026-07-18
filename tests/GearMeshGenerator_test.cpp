#include <gtest/gtest.h>
#include "GearMeshGenerator.hpp"
#include <glm/glm.hpp>
#include <cmath>

using namespace gear_engine;

TEST(GearMeshGeneratorTest, GeneratesCorrectVertexCount) {
    GearMeshGenerator generator;
    
    float radius = 1.0f;
    int num_teeth = 20;
    float pressure_angle = 20.0f; // New parameter
    float clearance = 0.1f;       // New parameter
    
    MeshData mesh = generator.Generate(radius, num_teeth, pressure_angle, clearance);
    
    EXPECT_GT(mesh.vertices.size(), 0);
    EXPECT_GT(mesh.indices.size(), 0);
}

TEST(GearMeshGeneratorTest, HandlesInvalidInputs) {
    GearMeshGenerator generator;
    
    MeshData mesh = generator.Generate(1.0f, 0, 20.0f, 0.1f); // 0 teeth
    
    EXPECT_EQ(mesh.vertices.size(), 0);
    EXPECT_EQ(mesh.indices.size(), 0);
}

TEST(GearMeshGeneratorTest, InvoluteMathCalculatesCorrectPoints) {
    float base_radius = 1.0f;
    // At t=0, the point should be on the base circle: (base_radius, 0)
    glm::vec2 p0 = involute::CalculatePoint(base_radius, 0.0f);
    EXPECT_NEAR(p0.x, base_radius, 0.001f);
    EXPECT_NEAR(p0.y, 0.0f, 0.001f);
    
    // At t=1.0, the angle is 1.0 radians. The curve unwinds.
    glm::vec2 p1 = involute::CalculatePoint(base_radius, 1.0f);
    // Standard involute parametric equations:
    // x = r_b * (cos(t) + t*sin(t))
    // y = r_b * (sin(t) - t*cos(t))
    float expected_x = base_radius * (std::cos(1.0f) + 1.0f * std::sin(1.0f));
    float expected_y = base_radius * (std::sin(1.0f) - 1.0f * std::cos(1.0f));
    EXPECT_NEAR(p1.x, expected_x, 0.001f);
    EXPECT_NEAR(p1.y, expected_y, 0.001f);
}

TEST(GearMeshGeneratorTest, GeneratesHelicalGear) {
    GearMeshGenerator generator;
    MeshData mesh = generator.Generate(1.0f, 20, 20.0f, 0.1f, GearType::Helical, 30.0f); // 30 deg helix angle
    EXPECT_GT(mesh.vertices.size(), 0);
    EXPECT_GT(mesh.indices.size(), 0);
}

TEST(GearMeshGeneratorTest, GeneratesBevelGear) {
    GearMeshGenerator generator;
    MeshData mesh = generator.Generate(1.0f, 20, 20.0f, 0.1f, GearType::Bevel, 45.0f); // 45 deg pitch angle
    EXPECT_GT(mesh.vertices.size(), 0);
    EXPECT_GT(mesh.indices.size(), 0);
}
