#pragma once
#include <stddef.h>
#include <stdint.h>

// Én telemetripakke. På radioen sendes den som kSize byte, little-endian, uten padding:
//
//  offset  type  felt
//       0  u8    version (= kVersion)
//       1  u8    phase (Phase)
//       2  u16   seq
//       4  u32   timeMs
//       8  u16   anomalies (Anomaly-bitflagg)
//      10  u8    flags: bit 0 = gpsValid, bit 1 = valveOpen
//      11  u8    gpsSatellites
//      12  f32   altitudeM
//      16  f32   velocityMs
//      20  f32   accelMs2
//      24  f32   chamberPressurePa
//      28  f32   tankPressurePa
//      32  f32   motorTempC
//      36  i32   lat (grader * 1e7)
//      40  i32   lon (grader * 1e7)
//      44  f32   gpsAltitudeM
//
// Bakkestasjonen dekoder med tools/telemetry_packet.py. Endres layouten, må kVersion økes og
// Python-filen endres likt.
struct TelemetryPacket {
    static constexpr uint8_t kVersion = 2;
    static constexpr size_t kSize = 48;  // RFM69 tar maks ca. 60 byte per pakke

    uint16_t seq = 0;     // øker med 1 per pakke, så bakken ser tapte pakker
    uint32_t timeMs = 0;  // flygecomputerens tid
    uint8_t phase = 0;
    uint16_t anomalies = 0;
    float altitudeM = 0;
    float velocityMs = 0;
    float accelMs2 = 0;
    float chamberPressurePa = 0;
    float tankPressurePa = 0;
    float motorTempC = 0;
    bool valveOpen = false;
    double latDeg = 0;
    double lonDeg = 0;
    float gpsAltitudeM = 0;
    uint8_t gpsSatellites = 0;
    bool gpsValid = false;

    // Skriver nøyaktig kSize byte til out.
    void encode(uint8_t* out) const;
    // Returnerer false hvis len er feil eller versjonen ikke stemmer.
    static bool decode(const uint8_t* in, size_t len, TelemetryPacket& out);
};
