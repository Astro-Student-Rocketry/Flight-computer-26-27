#pragma once
#include <vector>

#include "IRadio.h"

class FakeRadio : public IRadio {
public:
    bool begin() override { return beginOk; }
    bool send(const uint8_t* data, size_t len) override {
        if (!sendOk) return false;
        sendCount++;
        if (record) packets.emplace_back(data, data + len);
        return true;
    }

    int sendCount = 0;
    bool beginOk = true;
    bool sendOk = true;  // false = radioen er nede
    bool record = true;
    std::vector<std::vector<uint8_t>> packets;  // én oppføring per sendte pakke
};
