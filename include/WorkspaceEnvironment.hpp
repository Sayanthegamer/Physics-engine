#pragma once

namespace gear_engine {

class WorkspaceEnvironment {
public:
    WorkspaceEnvironment() : grid_enabled_(true), floor_enabled_(true) {}

    bool IsGridEnabled() const { return grid_enabled_; }
    void SetGridEnabled(bool enabled) { grid_enabled_ = enabled; }

    bool IsFloorEnabled() const { return floor_enabled_; }
    void SetFloorEnabled(bool enabled) { floor_enabled_ = enabled; }

private:
    bool grid_enabled_;
    bool floor_enabled_;
};

} // namespace gear_engine
