#pragma once

struct ImuSample {
    float ax = 0, ay = 0, az = 0;  // m/s^2
    float gx = 0, gy = 0, gz = 0;  // rad/s
};

class IImu {
public:
    virtual ~IImu() = default;
    virtual bool begin() = 0;
    virtual bool read(ImuSample& out) = 0;
};
