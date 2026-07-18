#include <gtest/gtest.h>
#include "GearEngine.hpp"

using namespace gear_engine;

TEST(EngineStateTest, StoresInvoluteProperties) {
    EngineState state;
    EntityHandle handle = state.AllocateBody();
    
    EXPECT_TRUE(handle.IsValid());
    uint32_t i = handle.index;
    
    // Assign properties
    state.GetBodies().pressure_angles[i] = 25.0f;
    state.GetBodies().clearances[i] = 0.2f;
    
    // Check properties
    EXPECT_FLOAT_EQ(state.GetBodies().pressure_angles[i], 25.0f);
    EXPECT_FLOAT_EQ(state.GetBodies().clearances[i], 0.2f);
}

TEST(EngineStateTest, StoresGearTypeAndGenericParams) {
    EngineState state;
    EntityHandle handle = state.AllocateBody();
    
    EXPECT_TRUE(handle.IsValid());
    uint32_t i = handle.index;
    
    // Assign generic gear properties
    state.GetBodies().gear_types[i] = GearType::Helical;
    state.GetBodies().gear_params_0[i] = 15.0f;
    state.GetBodies().gear_params_1[i] = 2.0f;
    
    // Check properties
    EXPECT_EQ(state.GetBodies().gear_types[i], GearType::Helical);
    EXPECT_FLOAT_EQ(state.GetBodies().gear_params_0[i], 15.0f);
    EXPECT_FLOAT_EQ(state.GetBodies().gear_params_1[i], 2.0f);
}

#include "ConstraintSolver.hpp"

TEST(ConstraintSolverTest, AutoDisconnectsWhenGearsMovedApart) {
    EngineState state;
    ConstraintSolver solver;
    ConstraintArrays constraints;
    
    EntityHandle a = state.AllocateBody();
    EntityHandle b = state.AllocateBody();
    
    // Set up initially close
    state.GetBodies().positions[a.index] = glm::vec3(0, 0, 0);
    state.GetBodies().positions[b.index] = glm::vec3(2, 0, 0);
    state.GetBodies().radii[a.index] = 1.0f;
    state.GetBodies().radii[b.index] = 1.0f;
    
    // Add constraint
    GearConstraint gc;
    gc.body_a = a;
    gc.body_b = b;
    constraints.gears.push_back(gc);
    
    EXPECT_EQ(constraints.gears.size(), 1);
    
    // Still close, should not disconnect
    solver.CheckAutoDisconnect(state, constraints, 0.5f);
    EXPECT_EQ(constraints.gears.size(), 1);
    
    // Move apart
    state.GetBodies().positions[b.index] = glm::vec3(10, 0, 0);
    
    // Should disconnect
    solver.CheckAutoDisconnect(state, constraints, 0.5f);
    EXPECT_EQ(constraints.gears.size(), 0);
}

TEST(ConstraintSolverTest, SolveGearsUses3DAxes) {
    EngineState state;
    ConstraintSolver solver;
    ConstraintArrays constraints;
    
    EntityHandle a = state.AllocateBody();
    EntityHandle b = state.AllocateBody();
    
    // Set up 3D rotations: a on X axis, b on Y axis
    state.GetBodies().angular_velocities[a.index] = glm::vec3(10.0f, 0.0f, 0.0f); // A rotating around X
    state.GetBodies().angular_velocities[b.index] = glm::vec3(0.0f, 0.0f, 0.0f); // B stationary
    
    state.GetBodies().inverse_inertias[a.index] = glm::mat3(1.0f); // Identity inertia
    state.GetBodies().inverse_inertias[b.index] = glm::mat3(1.0f);
    
    GearConstraint gc;
    gc.body_a = a;
    gc.body_b = b;
    gc.ratio = 1.0f; // 1:1 ratio
    gc.axis_a = glm::vec3(1.0f, 0.0f, 0.0f); // Meshing axis X
    gc.axis_b = glm::vec3(0.0f, 1.0f, 0.0f); // Meshing axis Y
    
    constraints.gears.push_back(gc);
    
    solver.Solve(state, constraints, 0.016f, 1);
    
    // A should slow down, B should speed up around Y axis (in negative direction)
    EXPECT_LT(state.GetBodies().angular_velocities[a.index].x, 10.0f);
    EXPECT_LT(state.GetBodies().angular_velocities[b.index].y, 0.0f);
    
    // Z velocities should remain 0
    EXPECT_FLOAT_EQ(state.GetBodies().angular_velocities[a.index].z, 0.0f);
    EXPECT_FLOAT_EQ(state.GetBodies().angular_velocities[b.index].z, 0.0f);
}

TEST(ConstraintSolverTest, SolveLinearGearUses3DAxes) {
    EngineState state;
    ConstraintSolver solver;
    ConstraintArrays constraints;
    
    EntityHandle pinion = state.AllocateBody();
    EntityHandle rack = state.AllocateBody();
    
    // Pinion rotating on Z axis, Rack translating on X axis
    state.GetBodies().angular_velocities[pinion.index] = glm::vec3(0.0f, 0.0f, 10.0f);
    state.GetBodies().linear_velocities[rack.index] = glm::vec3(0.0f, 0.0f, 0.0f);
    
    state.GetBodies().inverse_inertias[pinion.index] = glm::mat3(1.0f);
    state.GetBodies().inverse_masses[rack.index] = 1.0f;
    
    LinearGearConstraint lgc;
    lgc.body_rotational = pinion;
    lgc.body_linear = rack;
    lgc.ratio = 2.0f; // v = omega * r
    lgc.axis_rotational = glm::vec3(0.0f, 0.0f, 1.0f);
    lgc.axis_linear = glm::vec3(1.0f, 0.0f, 0.0f);
    
    constraints.linear_gears.push_back(lgc);
    
    solver.Solve(state, constraints, 0.016f, 1);
    
    // Pinion should slow down, rack should gain linear velocity on X (in negative direction)
    EXPECT_LT(state.GetBodies().angular_velocities[pinion.index].z, 10.0f);
    EXPECT_LT(state.GetBodies().linear_velocities[rack.index].x, 0.0f);
}

TEST(ConstraintSolverTest, WormGearNonBackdrivability) {
    EngineState state;
    ConstraintSolver solver;
    ConstraintArrays constraints;
    
    EntityHandle worm = state.AllocateBody();
    EntityHandle spur = state.AllocateBody();
    
    state.GetBodies().gear_types[worm.index] = GearType::Worm;
    state.GetBodies().gear_types[spur.index] = GearType::Spur;
    
    // Spur is trying to drive the worm
    state.GetBodies().angular_velocities[spur.index] = glm::vec3(0.0f, 0.0f, 10.0f);
    state.GetBodies().angular_velocities[worm.index] = glm::vec3(0.0f, 0.0f, 0.0f);
    
    state.GetBodies().inverse_inertias[worm.index] = glm::mat3(1.0f);
    state.GetBodies().inverse_inertias[spur.index] = glm::mat3(1.0f);
    
    GearConstraint gc;
    gc.body_a = worm;
    gc.body_b = spur;
    gc.ratio = 0.05f; // worm has small ratio
    gc.axis_a = glm::vec3(1.0f, 0.0f, 0.0f);
    gc.axis_b = glm::vec3(0.0f, 0.0f, 1.0f);
    
    constraints.gears.push_back(gc);
    
    solver.Solve(state, constraints, 0.016f, 1);
    
    // Worm should remain perfectly stationary (0.0f) because it cannot be backdriven.
    EXPECT_NEAR(state.GetBodies().angular_velocities[worm.index].x, 0.0f, 0.001f);
    // Spur should be stopped instantly because it hit an immovable object.
    EXPECT_NEAR(state.GetBodies().angular_velocities[spur.index].z, 0.0f, 0.001f);
}
