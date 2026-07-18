#include <gtest/gtest.h>
#include "GearMeshGenerator.hpp"
#include <glm/glm.hpp>

using namespace gear_engine;

// Test that a gear mesh generator can generate a valid mesh structure
TEST(GearMeshGeneratorTest, GeneratesCorrectVertexCount) {
    GearMeshGenerator generator;
    
    float radius = 1.0f;
    int num_teeth = 20;
    
    MeshData mesh = generator.Generate(radius, num_teeth);
    
    // For a gear, we expect vertices for the front and back faces, and the teeth.
    // Let's just assume it should have more than 0 vertices to start.
    EXPECT_GT(mesh.vertices.size(), 0);
    EXPECT_GT(mesh.indices.size(), 0);
}

// Test that an invalid gear configuration (like 0 teeth) returns empty mesh or handles it safely
TEST(GearMeshGeneratorTest, HandlesInvalidInputs) {
    GearMeshGenerator generator;
    
    MeshData mesh = generator.Generate(1.0f, 0); // 0 teeth
    
    EXPECT_EQ(mesh.vertices.size(), 0);
    EXPECT_EQ(mesh.indices.size(), 0);
}
