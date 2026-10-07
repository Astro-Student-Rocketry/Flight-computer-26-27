# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Flight computer firmware for the 26/27 rocket season, PlatformIO + Arduino on a Teensy 4.1. It reads sensors, decides the flight phase, logs to SD and sends telemetry. It controls nothing physically: recovery is a separate system and the motor is only monitored. `README.md` (Norwegian) is the spec: state diagram, transition table, safety rules, anomaly flags, per-tick pipeline, Luftfartstilsynet requirements, and a **pre-flight checklist of guessed thresholds**. Keep the README in sync when changing transitions, thresholds, anomalies or the tick pipeline.

Code comments, README and commit messages are in Norwegian. Write new comments in Norwegian to match.

## Commands

```sh
pio run                                   # build firmware (default env: teensy41)
pio run -t upload                         # flash Teensy
pio test -e native                        # run all unit tests on the host
pio test -e native -f test_apogee_detector  # run one test suite (folder name under test/)
cd tools && python3 -m unittest -v           # ground-station telemetry decoder tests
```

There is no way to run a single test case; each `test/test_*/test_main.cpp` is its own Unity program, so filter by suite and comment out `RUN_TEST` lines if needed. New test functions must be added to the `RUN_TEST` list in that file's `main()`.

## Architecture

Three libraries under `lib/`, linked by PlatformIO's library finder (includes are flat, e.g. `#include "VehicleState.h"`):

- `lib/hal/` — pure-virtual sensor/IO interfaces (`IBarometer`, `IImu`, `IGps`, `IPressureSensor`, `ITemperatureSensor`, `IValveSensor`, `IRadio`, `IStorage`).
- `lib/core/` — all flight logic, hardware-independent. Depends only on `hal/` interfaces, never on Arduino. Has no clock: time comes in as `timestampUs` / `nowUs` / `dt` from the caller.
- `lib/drivers/` — Arduino implementations of the `hal/` interfaces (BMP390, VN-100, GPS, RFM69, SD, transducers, thermocouple, valve switch). Currently mostly `// TODO` stubs.

The `native` env sets `lib_ignore = drivers` and `build_src_filter = -<*>`, so `src/main.cpp` and drivers are never compiled for tests. Anything in `core/` that pulls in Arduino headers will break the native build.

**Data flow per tick** (`FlightComputer::tick`, still a TODO): `HealthMonitor` → `StateEstimator` (baro/IMU/GPS → `VehicleState` + `RawSensorData`) → `MotorMonitor` (motor sensors → `MotorState`) → `FlightStateMachine::update(VehicleState, MotorState)` → `DataLogger` (SD, plus `Event` on phase change / new anomaly / rail exit) → `TelemetryEncoder` + `TelemetryLink`.

**Telemetry wire format** is defined in `lib/core/TelemetryPacket.h` (48 bytes, little-endian, explicit byte-by-byte encode, no `memcpy` of the struct) and mirrored in `tools/telemetry_packet.py`, which the ground station (separate repo `Ground_station_backend`, FastAPI) uses to turn one packet into a `telemetry` row and an `engine` row. Any layout change must bump `TelemetryPacket::kVersion`, update the Python `_STRUCT`, and regenerate the shared golden bytes in both `test/test_telemetry_packet` and `tools/test_telemetry_packet.py`. RFM69 payload limit is ~60 bytes. `FlightComputer` member declaration order is construction order and matters.

**State machine** (`FlightStateMachine`): phases in `Phase.h`. Ground phases Idle ↔ Filling → Filled, with Venting back to Idle; flight phases Liftoff → Boost → Coast → Descent → Landed are forward-only. Each transition is delegated to a detector class (`FillDetector`, `VentDetector`, `LiftoffDetector`, `ApogeeDetector`, `LandingDetector`), each with a nested `Config` struct of thresholds; `FlightStateMachine::Config` aggregates them plus timing. Design rules enforced in code and tests:
- Detectors count consecutive samples (never trust one reading), so every detector active in a phase must be called every tick. Evaluate them into locals first, don't put them inside a short-circuiting `||`.
- Liftoff is only accepted from `Filled`. No transitions backwards after liftoff.
- Every flight transition has a timeout fallback that raises an `Anomaly` bit flag (`Anomaly.h`, `uint16_t` mask).

Status: `FillDetector`, `VentDetector`, `LiftoffDetector`, `ApogeeDetector`, `MotorMonitor`, `FlightStateMachine`, `TelemetryPacket`, `TelemetryLink` are implemented and tested. `StateEstimator`, `LandingDetector`, `HealthMonitor`, `DataLogger`, `TelemetryEncoder`, `FlightComputer::tick` and all drivers are stubs.

## Tests

- `test/fakes/Fake*.h` — header-only fakes of each `hal/` interface with public fields to set values and `beginOk`/`readOk` to inject failures.
- `test/fakes/FlightSim.h` — deterministic simplified flight (Pad → Fill → Filled → Boost → Coast → Descent → Landed) that writes into the fakes each `step(dt)`; `runUntil(Stage)` fast-forwards. Numbers are plausible placeholders, not Simulink data.
- `test/helpers/TestHelpers.h` — `SimRig` bundles fakes + sim and builds `MotorState`; `vehicleFrom(rig)` builds `VehicleState` the way `StateEstimator` is meant to (subtracts g from IMU specific force); `ramp()` plays a noisy linear pressure ramp. Everything here is header-only because each test folder is a separate binary.
