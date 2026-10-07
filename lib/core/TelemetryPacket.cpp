#include "TelemetryPacket.h"

#include <math.h>
#include <string.h>

namespace {

constexpr uint8_t kFlagGpsValid = 1 << 0;
constexpr uint8_t kFlagValveOpen = 1 << 1;

// Skriver og leser byte for byte, så resultatet ikke avhenger av padding eller byte-rekkefølge.
class Writer {
public:
    explicit Writer(uint8_t* p) : p_(p) {}
    void u8(uint8_t v) { *p_++ = v; }
    void u16(uint16_t v) {
        u8(v & 0xFF);
        u8(v >> 8);
    }
    void u32(uint32_t v) {
        u16(v & 0xFFFF);
        u16(v >> 16);
    }
    void i32(int32_t v) { u32(static_cast<uint32_t>(v)); }
    void f32(float v) {
        uint32_t bits;
        memcpy(&bits, &v, sizeof(bits));
        u32(bits);
    }

private:
    uint8_t* p_;
};

class Reader {
public:
    explicit Reader(const uint8_t* p) : p_(p) {}
    uint8_t u8() { return *p_++; }
    uint16_t u16() {
        uint16_t lo = u8();
        return lo | static_cast<uint16_t>(u8()) << 8;
    }
    uint32_t u32() {
        uint32_t lo = u16();
        return lo | static_cast<uint32_t>(u16()) << 16;
    }
    int32_t i32() { return static_cast<int32_t>(u32()); }
    float f32() {
        uint32_t bits = u32();
        float v;
        memcpy(&v, &bits, sizeof(v));
        return v;
    }

private:
    const uint8_t* p_;
};

int32_t toE7(double deg) { return static_cast<int32_t>(lround(deg * 1e7)); }

}  // namespace

void TelemetryPacket::encode(uint8_t* out) const {
    Writer w(out);
    w.u8(kVersion);
    w.u8(phase);
    w.u16(seq);
    w.u32(timeMs);
    w.u16(anomalies);
    w.u8((gpsValid ? kFlagGpsValid : 0) | (valveOpen ? kFlagValveOpen : 0));
    w.u8(gpsSatellites);
    w.f32(altitudeM);
    w.f32(velocityMs);
    w.f32(accelMs2);
    w.f32(chamberPressurePa);
    w.f32(tankPressurePa);
    w.f32(motorTempC);
    w.i32(toE7(latDeg));
    w.i32(toE7(lonDeg));
    w.f32(gpsAltitudeM);
}

bool TelemetryPacket::decode(const uint8_t* in, size_t len, TelemetryPacket& out) {
    if (len != kSize || in[0] != kVersion) return false;
    Reader r(in + 1);
    TelemetryPacket p;
    p.phase = r.u8();
    p.seq = r.u16();
    p.timeMs = r.u32();
    p.anomalies = r.u16();
    const uint8_t flags = r.u8();
    p.gpsValid = flags & kFlagGpsValid;
    p.valveOpen = flags & kFlagValveOpen;
    p.gpsSatellites = r.u8();
    p.altitudeM = r.f32();
    p.velocityMs = r.f32();
    p.accelMs2 = r.f32();
    p.chamberPressurePa = r.f32();
    p.tankPressurePa = r.f32();
    p.motorTempC = r.f32();
    p.latDeg = r.i32() * 1e-7;
    p.lonDeg = r.i32() * 1e-7;
    p.gpsAltitudeM = r.f32();
    out = p;
    return true;
}
