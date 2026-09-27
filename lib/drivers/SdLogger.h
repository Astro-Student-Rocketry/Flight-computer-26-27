#pragma once
#include "IStorage.h"

class SdLogger : public IStorage {
public:
    bool begin() override;
    bool write(const uint8_t* data, size_t len) override;
    void flush() override;
};
