#pragma once

class HealthMonitor {
public:
    void update();
    bool isSafeToArm() const;
};
