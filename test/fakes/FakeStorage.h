#pragma once
#include <vector>

#include "IStorage.h"

class FakeStorage : public IStorage {
public:
    bool begin() override { return beginOk; }
    bool write(const uint8_t* data, size_t len) override {
        if (!writeOk) return false;
        bytesWritten += len;
        if (record) buffer.insert(buffer.end(), data, data + len);
        return true;
    }
    void flush() override { flushCount++; }

    size_t bytesWritten = 0;
    int flushCount = 0;
    bool beginOk = true;
    bool writeOk = true;  // false = SD-kortet feiler
    bool record = true;   // false sparer minne i lange simuleringer
    std::vector<uint8_t> buffer;  // alt som er skrevet, i rekkefølge
};
