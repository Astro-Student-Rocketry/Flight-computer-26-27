#include <unity.h>
#include <math.h>

#include "Anomaly.h"
#include "Phase.h"
#include "TelemetryLink.h"
#include "TelemetryPacket.h"
#include "../fakes/FakeRadio.h"

void setUp() {}
void tearDown() {}

// Samme byte som GOLDEN i tools/test_telemetry_packet.py. Feiler én av testene, er C++ og Python
// ikke enige om layouten.
static const uint8_t kGolden[TelemetryPacket::kSize] = {
    0x02, 0x05, 0x02, 0x01, 0x40, 0xE2, 0x01, 0x00, 0x11, 0x00, 0x03, 0x09,
    0x00, 0x50, 0x9A, 0x44, 0x00, 0x40, 0x7A, 0x43, 0x00, 0x00, 0x1C, 0xC1,
    0x00, 0x1B, 0x37, 0x4A, 0x40, 0x54, 0x89, 0x4A, 0x00, 0x00, 0xAC, 0x41,
    0x28, 0xFB, 0xFE, 0x23, 0x88, 0x16, 0x2C, 0x03, 0x00, 0xC0, 0x99, 0x44,
};

// Unity er bygget uten double-støtte. 1e-7 grader er ca. 1 cm.
void assertDegEqual(double expected, double actual) {
    TEST_ASSERT_TRUE(fabs(expected - actual) <= 1e-7);
}

TelemetryPacket goldenPacket() {
    TelemetryPacket p;
    p.seq = 0x0102;
    p.timeMs = 123456;
    p.phase = static_cast<uint8_t>(Phase::Boost);
    p.anomalies = static_cast<uint16_t>(Anomaly::EarlyBurnout) |
                  static_cast<uint16_t>(Anomaly::GpsLost);
    p.gpsValid = true;
    p.valveOpen = true;
    p.gpsSatellites = 9;
    p.altitudeM = 1234.5f;
    p.velocityMs = 250.25f;
    p.accelMs2 = -9.75f;
    p.chamberPressurePa = 3.0e6f;
    p.tankPressurePa = 4.5e6f;
    p.motorTempC = 21.5f;
    p.latDeg = 60.3913;
    p.lonDeg = 5.3221;
    p.gpsAltitudeM = 1230.0f;
    return p;
}

void test_packet_fits_rfm69() { TEST_ASSERT_TRUE(TelemetryPacket::kSize <= 60); }

void test_encode_matches_golden_bytes() {
    uint8_t buf[TelemetryPacket::kSize];
    goldenPacket().encode(buf);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(kGolden, buf, TelemetryPacket::kSize);
}

void test_decode_golden_bytes() {
    TelemetryPacket p;
    TEST_ASSERT_TRUE(TelemetryPacket::decode(kGolden, sizeof(kGolden), p));
    const TelemetryPacket e = goldenPacket();
    TEST_ASSERT_EQUAL_UINT16(e.seq, p.seq);
    TEST_ASSERT_EQUAL_UINT32(e.timeMs, p.timeMs);
    TEST_ASSERT_EQUAL_UINT8(e.phase, p.phase);
    TEST_ASSERT_EQUAL_UINT16(e.anomalies, p.anomalies);
    TEST_ASSERT_TRUE(p.gpsValid);
    TEST_ASSERT_TRUE(p.valveOpen);
    TEST_ASSERT_EQUAL_UINT8(e.gpsSatellites, p.gpsSatellites);
    TEST_ASSERT_EQUAL_FLOAT(e.altitudeM, p.altitudeM);
    TEST_ASSERT_EQUAL_FLOAT(e.velocityMs, p.velocityMs);
    TEST_ASSERT_EQUAL_FLOAT(e.accelMs2, p.accelMs2);
    TEST_ASSERT_EQUAL_FLOAT(e.chamberPressurePa, p.chamberPressurePa);
    TEST_ASSERT_EQUAL_FLOAT(e.tankPressurePa, p.tankPressurePa);
    TEST_ASSERT_EQUAL_FLOAT(e.motorTempC, p.motorTempC);
    TEST_ASSERT_EQUAL_FLOAT(e.gpsAltitudeM, p.gpsAltitudeM);
    assertDegEqual(e.latDeg, p.latDeg);
    assertDegEqual(e.lonDeg, p.lonDeg);
}

void test_negative_coordinates_round_trip() {
    TelemetryPacket in;
    in.latDeg = -33.8688197;
    in.lonDeg = -151.2092955;
    uint8_t buf[TelemetryPacket::kSize];
    in.encode(buf);
    TelemetryPacket out;
    TEST_ASSERT_TRUE(TelemetryPacket::decode(buf, sizeof(buf), out));
    assertDegEqual(in.latDeg, out.latDeg);
    assertDegEqual(in.lonDeg, out.lonDeg);
}

void test_decode_rejects_wrong_length() {
    TelemetryPacket p;
    TEST_ASSERT_FALSE(TelemetryPacket::decode(kGolden, sizeof(kGolden) - 1, p));
}

void test_decode_rejects_wrong_version() {
    uint8_t buf[TelemetryPacket::kSize];
    goldenPacket().encode(buf);
    buf[0] = 1;
    TelemetryPacket p;
    TEST_ASSERT_FALSE(TelemetryPacket::decode(buf, sizeof(buf), p));
}

void test_link_sends_encoded_packet() {
    FakeRadio radio;
    TelemetryLink link(radio);
    TEST_ASSERT_TRUE(link.send(goldenPacket()));
    TEST_ASSERT_EQUAL(1, radio.packets.size());
    TEST_ASSERT_EQUAL(TelemetryPacket::kSize, radio.packets[0].size());
    TEST_ASSERT_EQUAL_HEX8_ARRAY(kGolden, radio.packets[0].data(), TelemetryPacket::kSize);
}

void test_link_reports_radio_failure() {
    FakeRadio radio;
    radio.sendOk = false;
    TelemetryLink link(radio);
    TEST_ASSERT_FALSE(link.send(goldenPacket()));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_packet_fits_rfm69);
    RUN_TEST(test_encode_matches_golden_bytes);
    RUN_TEST(test_decode_golden_bytes);
    RUN_TEST(test_negative_coordinates_round_trip);
    RUN_TEST(test_decode_rejects_wrong_length);
    RUN_TEST(test_decode_rejects_wrong_version);
    RUN_TEST(test_link_sends_encoded_packet);
    RUN_TEST(test_link_reports_radio_failure);
    return UNITY_END();
}
