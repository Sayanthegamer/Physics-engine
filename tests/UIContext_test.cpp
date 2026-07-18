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

TEST(UIContextTest, GizmoStateManagement) {
    UIContext ui_ctx;
    
    // Default mode should be Translate
    EXPECT_EQ(ui_ctx.GetGizmoMode(), GizmoMode::Translate);
    
    ui_ctx.SetGizmoMode(GizmoMode::Rotate);
    EXPECT_EQ(ui_ctx.GetGizmoMode(), GizmoMode::Rotate);
    
    ui_ctx.SetGizmoMode(GizmoMode::Scale);
    EXPECT_EQ(ui_ctx.GetGizmoMode(), GizmoMode::Scale);
}

TEST(UIContextTest, ObjectSelectionManagement) {
    UIContext ui_ctx;
    
    // No object selected by default
    EXPECT_FALSE(ui_ctx.HasSelectedObject());
    EXPECT_EQ(ui_ctx.GetSelectedObjectId(), 0); // Assuming 0 is invalid/null ID
    
    ui_ctx.SetSelectedObjectId(42);
    EXPECT_TRUE(ui_ctx.HasSelectedObject());
    EXPECT_EQ(ui_ctx.GetSelectedObjectId(), 42);
    
    ui_ctx.ClearSelection();
    EXPECT_FALSE(ui_ctx.HasSelectedObject());
}
