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
    }

private:
    GearType selected_gear_type_ = GearType::Spur;
};

} // namespace gear_engine
