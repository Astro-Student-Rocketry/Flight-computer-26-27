"""Dekoder telemetripakker fra flygecomputeren.

Layouten er definert i lib/core/TelemetryPacket.h. Endres den, må VERSION og _STRUCT her endres likt.

Én radiopakke blir én rad i telemetry og én rad i engine i Ground_station_backend:

    pkt = TelemetryPacket.decode(data)
    telemetry = pkt.to_telemetry(received_at=now_ms, rssi_dbm=rssi)  # passer TelemetryCreate
    engine = pkt.to_engine(received_at=now_ms)                        # passer EngineDataCreate
"""

from __future__ import annotations

import struct
from dataclasses import dataclass

VERSION = 2

# Little-endian, ingen padding. Samme rekkefølge som tabellen i TelemetryPacket.h.
_STRUCT = struct.Struct("<BBHIHBBffffffiif")
SIZE = _STRUCT.size

# Samme rekkefølge som enum class Phase i lib/core/Phase.h.
PHASES = ("Idle", "Filling", "Filled", "Venting", "Liftoff", "Boost", "Coast", "Descent", "Landed")

# Samme bits som enum class Anomaly i lib/core/Anomaly.h.
ANOMALIES = {
    1 << 0: "EarlyBurnout",
    1 << 1: "ApogeeByTimeout",
    1 << 2: "LandingByTimeout",
    1 << 3: "SensorFault",
    1 << 4: "GpsLost",
    1 << 5: "BurnoutByTimeout",
}

_FLAG_GPS_VALID = 1 << 0
_FLAG_VALVE_OPEN = 1 << 1

# Bits i engine.valve_state.
VALVE_OPEN = 1 << 0


@dataclass
class TelemetryPacket:
    seq: int
    time_ms: int
    phase: int
    anomalies: int
    gps_valid: bool
    valve_open: bool
    gps_satellites: int
    altitude_m: float
    velocity_ms: float
    accel_ms2: float
    chamber_pressure_pa: float
    tank_pressure_pa: float
    motor_temp_c: float
    lat_deg: float
    lon_deg: float
    gps_altitude_m: float

    @classmethod
    def decode(cls, data: bytes) -> TelemetryPacket:
        if len(data) != SIZE:
            raise ValueError(f"feil lengde: {len(data)} byte, ventet {SIZE}")
        (version, phase, seq, time_ms, anomalies, flags, sats, alt, vel, acc, chamber, tank,
         temp, lat_e7, lon_e7, gps_alt) = _STRUCT.unpack(data)
        if version != VERSION:
            raise ValueError(f"ukjent versjon {version}, ventet {VERSION}")
        return cls(
            seq=seq,
            time_ms=time_ms,
            phase=phase,
            anomalies=anomalies,
            gps_valid=bool(flags & _FLAG_GPS_VALID),
            valve_open=bool(flags & _FLAG_VALVE_OPEN),
            gps_satellites=sats,
            altitude_m=alt,
            velocity_ms=vel,
            accel_ms2=acc,
            chamber_pressure_pa=chamber,
            tank_pressure_pa=tank,
            motor_temp_c=temp,
            lat_deg=lat_e7 * 1e-7,
            lon_deg=lon_e7 * 1e-7,
            gps_altitude_m=gps_alt,
        )

    @property
    def phase_name(self) -> str:
        return PHASES[self.phase] if self.phase < len(PHASES) else f"Unknown({self.phase})"

    @property
    def anomaly_names(self) -> list[str]:
        return [name for bit, name in ANOMALIES.items() if self.anomalies & bit]

    def to_telemetry(self, received_at: int, rssi_dbm: int | None = None) -> dict:
        """Felter for TelemetryCreate. Det flygecomputeren ikke måler, blir None."""
        gps = self.gps_valid
        return {
            "seq": self.seq,
            "t_ms": self.time_ms,
            "received_at": received_at,
            "flight_state": self.phase_name,
            "altitude_m": self.altitude_m,
            "velocity_ms": self.velocity_ms,
            "accel_ms2": self.accel_ms2,
            "gps_lat": self.lat_deg if gps else None,
            "gps_lon": self.lon_deg if gps else None,
            "gps_alt_m": self.gps_altitude_m if gps else None,
            "rssi_dbm": rssi_dbm,
            # Mangler i TelemetryCreate foreløpig. Pydantic ignorerer dem til bakkestasjonen har
            # kolonner for dem.
            "anomalies": self.anomalies,
            "gps_satellites": self.gps_satellites,
        }

    def to_engine(self, received_at: int) -> dict:
        """Felter for EngineDataCreate."""
        return {
            "t_ms": self.time_ms,
            "received_at": received_at,
            "chamber_pressure_bar": self.chamber_pressure_pa / 1e5,
            "tank_pressure_bar": self.tank_pressure_pa / 1e5,
            # TODO: termoelementets plassering er ikke bestemt. Antar at det sitter på tanken.
            "tank_temp_c": self.motor_temp_c,
            "valve_state": VALVE_OPEN if self.valve_open else 0,
        }
