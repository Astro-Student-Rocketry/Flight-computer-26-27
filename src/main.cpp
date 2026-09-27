#include <Arduino.h>

#include "Bmp390.h"
#include "FlightComputer.h"
#include "GpsReceiver.h"
#include "PressureTransducer.h"
#include "Rfm69Link.h"
#include "SdLogger.h"
#include "Thermocouple.h"
#include "ValveSwitch.h"
#include "Vn100.h"

static Bmp390 baro;
static Vn100 imu;
static GpsReceiver gps;
static SdLogger sd;
static Rfm69Link radio;

// TODO: sett riktige pinner
static PressureTransducer chamberPressure(A0);
static PressureTransducer tankPressure(A1);
static Thermocouple motorTemp(10);
static ValveSwitch valve(2);

static FlightComputer fc(baro, imu, gps, sd, radio, chamberPressure, tankPressure, motorTemp, valve);

void setup() {
    Serial.begin(115200);
    fc.begin();
    // TODO: håndter init-feil
}

void loop() {
    // TODO: fast steglengde, og kall fc.tick(dt)
}
