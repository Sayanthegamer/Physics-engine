#include <gtest/gtest.h>
#include "WorkspaceEnvironment.hpp"

using namespace gear_engine;

TEST(WorkspaceEnvironmentTest, DefaultConfigHasGridAndFloorEnabled) {
    WorkspaceEnvironment env;
    
    EXPECT_TRUE(env.IsGridEnabled());
    EXPECT_TRUE(env.IsFloorEnabled());
    EXPECT_FALSE(env.IsWireframeEnabled());
}

TEST(WorkspaceEnvironmentTest, CanToggleGridAndFloor) {
    WorkspaceEnvironment env;
    
    env.SetGridEnabled(false);
    EXPECT_FALSE(env.IsGridEnabled());
    
    env.SetFloorEnabled(false);
    EXPECT_FALSE(env.IsFloorEnabled());
    
    env.SetGridEnabled(true);
    EXPECT_TRUE(env.IsGridEnabled());

    env.SetWireframeEnabled(true);
    EXPECT_TRUE(env.IsWireframeEnabled());
}
