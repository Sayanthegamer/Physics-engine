#pragma once

#include "../GearEngine.hpp"

namespace gear_engine {

enum class GizmoMode {
    Translate,
    Rotate,
    Scale
};

class UIContext {
public:
    UIContext() = default;

    GearType GetSelectedGearType() const {
        return selected_gear_type_;
    }

    void SetSelectedGearType(GearType type) {
        selected_gear_type_ = type;
        if (type == GearType::Helical) gear_param0_ = 30.0f;
        else if (type == GearType::Bevel) gear_param0_ = 45.0f;
        else if (type == GearType::Worm) gear_param0_ = 5.0f;
        else if (type == GearType::Rack) gear_param0_ = 10.0f;
        else if (type == GearType::Internal) gear_param0_ = 0.5f;
    }

    float GetGearParam0() const { return gear_param0_; }
    void SetGearParam0(float p) { gear_param0_ = p; }
    
    // Gizmo State
    GizmoMode GetGizmoMode() const { return gizmo_mode_; }
    void SetGizmoMode(GizmoMode mode) { gizmo_mode_ = mode; }
    
    // Object Selection
    bool HasSelectedObject() const { return selected_object_id_ != 0; }
    uint32_t GetSelectedObjectId() const { return selected_object_id_; }
    void SetSelectedObjectId(uint32_t id) { selected_object_id_ = id; }
    void ClearSelection() { selected_object_id_ = 0; }

private:
    GearType selected_gear_type_ = GearType::Spur;
    float gear_param0_ = 30.0f;
    GizmoMode gizmo_mode_ = GizmoMode::Translate;
    uint32_t selected_object_id_ = 0;
};

} // namespace gear_engine
