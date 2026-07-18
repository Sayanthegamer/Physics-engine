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
