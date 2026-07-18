#include <gtest/gtest.h>
#include "UI/UIContext.hpp"
#include "GearEngine.hpp"

using namespace gear_engine;

TEST(UIContextTest, DefaultSelectedGearIsSpur) {
    UIContext ui_ctx;
    EXPECT_EQ(ui_ctx.GetSelectedGearType(), GearType::Spur);
}

TEST(UIContextTest, CanSelectDifferentGearTypes) {
    UIContext ui_ctx;
    
    ui_ctx.SetSelectedGearType(GearType::Helical);
    EXPECT_EQ(ui_ctx.GetSelectedGearType(), GearType::Helical);
    
    ui_ctx.SetSelectedGearType(GearType::Bevel);
    EXPECT_EQ(ui_ctx.GetSelectedGearType(), GearType::Bevel);
    
    ui_ctx.SetSelectedGearType(GearType::Worm);
    EXPECT_EQ(ui_ctx.GetSelectedGearType(), GearType::Worm);
    
    ui_ctx.SetSelectedGearType(GearType::Rack);
    EXPECT_EQ(ui_ctx.GetSelectedGearType(), GearType::Rack);
    
    ui_ctx.SetSelectedGearType(GearType::Internal);
    EXPECT_EQ(ui_ctx.GetSelectedGearType(), GearType::Internal);
}

TEST(UIContextTest, SetsDefaultGearParam0WhenTypeChanges) {
    UIContext ui_ctx;
    
    ui_ctx.SetSelectedGearType(GearType::Helical);
    EXPECT_EQ(ui_ctx.GetGearParam0(), 30.0f);
    
    ui_ctx.SetSelectedGearType(GearType::Bevel);
    EXPECT_EQ(ui_ctx.GetGearParam0(), 45.0f);
    
    ui_ctx.SetSelectedGearType(GearType::Worm);
    EXPECT_EQ(ui_ctx.GetGearParam0(), 5.0f);
    
    ui_ctx.SetSelectedGearType(GearType::Rack);
    EXPECT_EQ(ui_ctx.GetGearParam0(), 10.0f);
    
    ui_ctx.SetSelectedGearType(GearType::Internal);
    EXPECT_EQ(ui_ctx.GetGearParam0(), 0.5f);
    
    ui_ctx.SetGearParam0(25.0f);
    EXPECT_EQ(ui_ctx.GetGearParam0(), 25.0f);
}
