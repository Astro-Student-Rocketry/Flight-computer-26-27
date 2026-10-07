#pragma once
#include <stddef.h>
#include <stdint.h>

class IStorage {
public:
    virtual ~IStorage() = default;
    virtual bool begin() = 0;
    virtual bool write(const uint8_t* data, size_t len) = 0;
    virtual void flush() = 0;
};
