#include <Arduino.h>

#include "Bmp390.h"
#include "FlightComputer.h"
#include "PyroDriver.h"
#include "Rfm69Link.h"
#include "SdLogger.h"
#include "Vn100.h"

static Bmp390 baro;
static Vn100 imu;
static SdLogger sd;
static Rfm69Link radio;
static PyroDriver pyro;
static FlightComputer fc(baro, imu, sd, radio, pyro);

void setup() {
    Serial.begin(115200);
    fc.begin();
    // TODO: håndter init-feil
}

void loop() {
    // TODO: fast steglengde, og kall fc.tick(dt)
}
