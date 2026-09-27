#pragma once
#include "IStorage.h"

class FakeStorage : public IStorage {
public:
    bool begin() override { return true; }
    bool write(const uint8_t* data, size_t len) override {
        (void)data;
        bytesWritten += len;
        return true;
    }
    void flush() override {}

    size_t bytesWritten = 0;
};
