# Flight-computer-26-27

Flygecomputer for rakettsesongen 26/27. Den leser sensorer, bestemmer hvilken fase raketten er i, logger alt til SD-kort og sender telemetri til bakken. Den styrer ingenting fysisk: recovery er et separat system, og motoren bare overvåkes.

## Tilstandsmaskin

`FlightStateMachine` bestemmer fasen ut fra `VehicleState` (høyde, fart, akselerasjon) og `MotorState` (tanktrykk, kammertrykk, temperatur, ventil).

```mermaid
stateDiagram-v2
    [*] --> Idle
    Idle --> Filling: isFilling
    Filling --> Filled: isFull
    Filling --> Venting: isVenting
    Filled --> Venting: isVenting
    Venting --> Idle: isEmpty
    Filled --> Liftoff: liftoff
    Liftoff --> Boost: bekreftet
    Boost --> Coast: burnout
    Coast --> Descent: apogee
    Descent --> Landed: landed
    Landed --> [*]
```

### Overganger

| Fra → til | Betingelse | Kilde |
|---|---|---|
| Idle → Filling | Tanktrykket stiger over omgivelsestrykk + margin | `FillDetector::isFilling` |
| Filling → Filled | Tanktrykket er over måltrykk og stabilt i noen sekunder | `FillDetector::isFull` |
| Filling/Filled → Venting | Tanktrykket faller jevnt | `VentDetector::isVenting` |
| Venting → Idle | Tanktrykket er tilbake nær omgivelsestrykk | `VentDetector::isEmpty` |
| Filled → Liftoff | Akselerasjon over terskel i N prøver på rad | `LiftoffDetector` |
| Liftoff → Boost | Kort tid etter liftoff (bekrefter liftoff) | Tid i fasen |
| Boost → Coast | Kammertrykket faller under terskel, eller akselerasjonen blir negativ | `MotorState` / `VehicleState` |
| Coast → Descent | Farten er negativ i N prøver og høyden har falt fra toppen | `ApogeeDetector` |
| Descent → Landed | Farten ≈ 0 og høyden stabil i flere sekunder | `LandingDetector` |

Tersklene er ikke bestemt ennå. De skal justeres mot Simulink-modellen og motordataene.

### Sikkerhetsregler

1. **Ingen vei tilbake etter liftoff.** Fra `Liftoff` og framover går fasen bare framover.
2. **Liftoff krever `Filled`.** Støt på rampa under fylling kan ikke utløse liftoff.
3. **Aldri stol på én måling.** Alle detektorene krever at betingelsen er sann flere prøver på rad.
4. **Reserve-tidsgrenser.** Hvis en detektor aldri trigger, tvinger en tidsgrense overgangen.
5. **Faseendringer logges alltid** med tidsstempel.

### Avvik

`FlightStateMachine` holder et sett med avviksflagg (`Anomaly`) som logges og sendes med telemetrien:

| Flagg | Betyr |
|---|---|
| `EarlyBurnout` | Burnout kom tidligere enn forventet |
| `ApogeeByTimeout` | Coast → Descent skjedde via tidsgrense, ikke `ApogeeDetector` |
| `LandingByTimeout` | Descent → Landed skjedde via tidsgrense, ikke `LandingDetector` |
| `SensorFault` | En sensor ga ugyldige verdier eller svarte ikke |
| `GpsLost` | Mistet GPS-posisjon under flyturen |

## Hva som skjer i hver tick

`FlightComputer::tick(dt)` kjøres med fast frekvens:

1. `HealthMonitor` sjekker sensorer og batteri
2. `StateEstimator` leser barometer, IMU og GPS, og lager `VehicleState` og `RawSensorData`
3. `MotorMonitor` lager `MotorState` fra motorsensorene
4. `FlightStateMachine` bestemmer fasen og setter eventuelle avvik
5. `DataLogger` skriver estimater og rådata til SD, og skriver en `Event` ved faseendring, nytt avvik og rampeslutt
6. `TelemetryEncoder` og `TelemetryLink` sender telemetri med fase, avvik og GPS-posisjon

## Krav fra Luftfartstilsynet

[Veiledning for oppskyting av avanserte raketter](https://www.luftfartstilsynet.no/romfart/veiledninger-romfart/veiledning-for-oppskyting-av-avanserte-raketter/) stiller ingen direkte krav til flygecomputeren. Men rapporten etter flyturen og sikkerhetsdokumentasjonen krever data som bare flygecomputeren kan levere:

| Krav i veiledningen | Hvordan flygecomputeren dekker det |
|---|---|
| Data fra telemetri eller sporing som bekrefter faktisk bane og maksimal høyde | Høyde, fart og GPS-posisjon logges og sendes som telemetri |
| Søk etter raketten/rakettdelene etter oppskyting | GPS-posisjon i telemetrien |
| Hendelseslogg: kronologisk oversikt over operasjonen | `Event` for hver faseendring, avvik og rampeslutt, med tidsstempel |
| Beskrivelse av avvik fra plan | `Anomaly`-flagg |
| Validering av modeller og simuleringer | `RawSensorData` logges ved siden av estimatene, så flyturen kan sammenlignes med Simulink |
| Farer som skal vurderes: feiltenning, tidlig cutoff | Feiltenning: fasen blir stående i `Filled` til tanken luftes (`Venting`). Tidlig cutoff: `EarlyBurnout` |
| Avbruddsprosedyrer, inkludert tømming av drivstoff | `Venting`-fasen |
| Raketten må forlate rampa med minst 12–15 m/s | Hastigheten ved rampeslutt logges som `Event` (`RailExit`) |

Tennsystemet (sikkerhetsnøkkel, dead-man's switch, 60 sekunders venting ved feiltenning) er bakkeutstyr og ikke en del av denne koden.

## Mappestruktur

```
lib/hal/       grensesnitt mot maskinvare (IBarometer, IImu, IGps, IPressureSensor, ...)
lib/core/      all fly-logikk, uten Arduino-kode
lib/drivers/   drivere for ekte maskinvare (Bmp390, Vn100, GpsReceiver, SdLogger, ...)
src/main.cpp   kobler driverne inn i FlightComputer
test/          enhetstester med falsk maskinvare (test/fakes/)
```

`core/` avhenger bare av grensesnittene i `hal/`. Derfor kan all logikken testes på PC-en, også med data fra Simulink.

## Bygge og teste

Krever [PlatformIO](https://platformio.org/).

Kjør testene på PC-en:

```bash
pio test -e native
```

Bygg for flygecomputeren:

```bash
pio run -e teensy41
```
