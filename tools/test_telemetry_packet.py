import unittest

from telemetry_packet import SIZE, TelemetryPacket

# Samme byte som kGolden i test/test_telemetry_packet/test_main.cpp.
GOLDEN = bytes([
    0x02, 0x05, 0x02, 0x01, 0x40, 0xE2, 0x01, 0x00, 0x11, 0x00, 0x03, 0x09,
    0x00, 0x50, 0x9A, 0x44, 0x00, 0x40, 0x7A, 0x43, 0x00, 0x00, 0x1C, 0xC1,
    0x00, 0x1B, 0x37, 0x4A, 0x40, 0x54, 0x89, 0x4A, 0x00, 0x00, 0xAC, 0x41,
    0x28, 0xFB, 0xFE, 0x23, 0x88, 0x16, 0x2C, 0x03, 0x00, 0xC0, 0x99, 0x44,
])


class TestTelemetryPacket(unittest.TestCase):
    def test_size(self):
        self.assertEqual(SIZE, 48)

    def test_decode_golden(self):
        p = TelemetryPacket.decode(GOLDEN)
        self.assertEqual(p.seq, 0x0102)
        self.assertEqual(p.time_ms, 123456)
        self.assertEqual(p.phase_name, "Boost")
        self.assertEqual(p.anomaly_names, ["EarlyBurnout", "GpsLost"])
        self.assertTrue(p.gps_valid)
        self.assertTrue(p.valve_open)
        self.assertEqual(p.gps_satellites, 9)
        self.assertEqual(p.altitude_m, 1234.5)
        self.assertEqual(p.velocity_ms, 250.25)
        self.assertEqual(p.accel_ms2, -9.75)
        self.assertEqual(p.chamber_pressure_pa, 3.0e6)
        self.assertEqual(p.tank_pressure_pa, 4.5e6)
        self.assertEqual(p.motor_temp_c, 21.5)
        self.assertAlmostEqual(p.lat_deg, 60.3913, delta=1e-7)
        self.assertAlmostEqual(p.lon_deg, 5.3221, delta=1e-7)
        self.assertEqual(p.gps_altitude_m, 1230.0)

    def test_to_telemetry(self):
        t = TelemetryPacket.decode(GOLDEN).to_telemetry(received_at=1_700_000_000_000, rssi_dbm=-80)
        self.assertEqual(t["flight_state"], "Boost")
        self.assertEqual(t["t_ms"], 123456)
        self.assertEqual(t["rssi_dbm"], -80)
        self.assertAlmostEqual(t["gps_lat"], 60.3913, delta=1e-7)

    def test_to_telemetry_hides_invalid_gps(self):
        data = bytearray(GOLDEN)
        data[10] &= ~0x01
        t = TelemetryPacket.decode(bytes(data)).to_telemetry(received_at=0)
        self.assertIsNone(t["gps_lat"])
        self.assertIsNone(t["gps_lon"])
        self.assertIsNone(t["gps_alt_m"])

    def test_to_engine_converts_to_bar(self):
        e = TelemetryPacket.decode(GOLDEN).to_engine(received_at=0)
        self.assertEqual(e["chamber_pressure_bar"], 30.0)
        self.assertEqual(e["tank_pressure_bar"], 45.0)
        self.assertEqual(e["valve_state"], 1)

    def test_rejects_wrong_length(self):
        with self.assertRaises(ValueError):
            TelemetryPacket.decode(GOLDEN[:-1])

    def test_rejects_wrong_version(self):
        with self.assertRaises(ValueError):
            TelemetryPacket.decode(b"\x01" + GOLDEN[1:])


if __name__ == "__main__":
    unittest.main()
