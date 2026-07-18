#pragma once

#include "../GearEngine.hpp"

namespace gear_engine {

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

private:
    GearType selected_gear_type_ = GearType::Spur;
    float gear_param0_ = 30.0f;
};

} // namespace gear_engine
