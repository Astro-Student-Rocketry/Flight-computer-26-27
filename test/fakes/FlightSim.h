#pragma once
#include <cmath>

#include "FakeBarometer.h"
#include "FakeGps.h"
#include "FakeImu.h"
#include "FakePressureSensor.h"
#include "FakeTemperatureSensor.h"
#include "FakeValveSensor.h"

// Spiller av en forenklet flyging og setter verdiene i fakene for hvert tidssteg.
// Brukes til å teste detektorer og tilstandsmaskin uten maskinvare.
// Tallene er plausible, ikke fra Simulink. Bytt ut med ekte data når de finnes.
class FlightSim {
public:
    enum class Stage { Pad, Fill, Filled, Boost, Coast, Descent, Landed };

    struct Profile {
        float padTime = 5.0f;        // s stille på rampa før fylling
        float fillTime = 20.0f;      // s fra omgivelsestrykk til måltrykk
        float filledTime = 10.0f;    // s ferdig fylt, ventil lukket
        float burnTime = 3.0f;       // s
        float boostAccel = 80.0f;    // m/s^2 (kinematisk akselerasjon)
        float descentSpeed = 20.0f;  // m/s ned under fallskjerm
        float tankFullPa = 5.0e6f;
        float chamberPa = 3.0e6f;
        float groundPa = 101325.0f;
        float railLength = 5.0f;     // m
    };

    FlightSim(FakeBarometer& baro, FakeImu& imu, FakeGps& gps, FakePressureSensor& chamber,
              FakePressureSensor& tank, FakeTemperatureSensor& temp, FakeValveSensor& valve)
        : baro_(baro), imu_(imu), gps_(gps), chamber_(chamber), tank_(tank), temp_(temp),
          valve_(valve) {
        apply();
    }

    FlightSim(FakeBarometer& baro, FakeImu& imu, FakeGps& gps, FakePressureSensor& chamber,
              FakePressureSensor& tank, FakeTemperatureSensor& temp, FakeValveSensor& valve,
              const Profile& profile)
        : FlightSim(baro, imu, gps, chamber, tank, temp, valve) {
        p_ = profile;
        apply();
    }

    // Flytter simuleringen dt sekunder fram og oppdaterer fakene.
    void step(float dt) {
        t_ += dt;
        stageTime_ += dt;
        switch (stage_) {
            case Stage::Pad:
                if (stageTime_ >= p_.padTime) enter(Stage::Fill);
                break;
            case Stage::Fill:
                if (stageTime_ >= p_.fillTime) enter(Stage::Filled);
                break;
            case Stage::Filled:
                if (stageTime_ >= p_.filledTime) enter(Stage::Boost);
                break;
            case Stage::Boost:
                vel_ += p_.boostAccel * dt;
                alt_ += vel_ * dt;
                if (stageTime_ >= p_.burnTime) enter(Stage::Coast);
                break;
            case Stage::Coast:
                vel_ -= kG * dt;
                alt_ += vel_ * dt;
                if (vel_ <= -p_.descentSpeed) enter(Stage::Descent);
                break;
            case Stage::Descent:
                vel_ = -p_.descentSpeed;
                alt_ += vel_ * dt;
                if (alt_ <= 0.0f) {
                    alt_ = 0.0f;
                    vel_ = 0.0f;
                    enter(Stage::Landed);
                }
                break;
            case Stage::Landed:
                break;
        }
        if (alt_ > apogee_) apogee_ = alt_;
        apply();
    }

    // Kjører til en gitt fase er nådd. Returnerer false hvis det tar over maxSeconds.
    bool runUntil(Stage target, float dt = 0.01f, float maxSeconds = 600.0f) {
        while (stage_ != target) {
            if (t_ > maxSeconds) return false;
            step(dt);
        }
        return true;
    }

    Stage stage() const { return stage_; }
    float time() const { return t_; }
    float altitude() const { return alt_; }
    float velocity() const { return vel_; }
    float apogee() const { return apogee_; }
    float railLength() const { return p_.railLength; }

private:
    static constexpr float kG = 9.81f;

    void enter(Stage s) {
        stage_ = s;
        stageTime_ = 0.0f;
    }

    // Standardatmosfære, gyldig langt over det raketten når.
    float pressureAt(float altM) const {
        return p_.groundPa * std::pow(1.0f - altM / 44330.0f, 5.255f);
    }

    void apply() {
        baro_.pressure = pressureAt(alt_);

        // IMU måler spesifikk kraft: ca. +g i ro, 0 i fritt fall.
        float az = kG;
        if (stage_ == Stage::Boost) az = p_.boostAccel + kG;
        if (stage_ == Stage::Coast) az = 0.0f;
        imu_.sample.az = az;

        float tank = p_.groundPa;
        float chamber = p_.groundPa;
        bool valveOpen = false;
        const float frac = stageTime_ / (stage_ == Stage::Fill ? p_.fillTime : 1.0f);
        switch (stage_) {
            case Stage::Fill:
                tank = p_.groundPa + (p_.tankFullPa - p_.groundPa) * (frac > 1.0f ? 1.0f : frac);
                valveOpen = true;
                break;
            case Stage::Filled:
                tank = p_.tankFullPa;
                break;
            case Stage::Boost: {
                float burn = stageTime_ / p_.burnTime;
                tank = p_.tankFullPa * (1.0f - 0.8f * (burn > 1.0f ? 1.0f : burn));
                chamber = p_.chamberPa;
                break;
            }
            case Stage::Coast:
            case Stage::Descent:
            case Stage::Landed:
                tank = p_.groundPa;
                break;
            default:
                break;
        }
        tank_.pressure = tank;
        chamber_.pressure = chamber;
        valve_.open = valveOpen;
        temp_.temp = stage_ == Stage::Boost ? 120.0f : 15.0f;

        gps_.fix.altitudeM = alt_;
        gps_.fix.valid = true;
        gps_.fix.satellites = 9;
    }

    FakeBarometer& baro_;
    FakeImu& imu_;
    FakeGps& gps_;
    FakePressureSensor& chamber_;
    FakePressureSensor& tank_;
    FakeTemperatureSensor& temp_;
    FakeValveSensor& valve_;

    Profile p_;
    Stage stage_ = Stage::Pad;
    float t_ = 0.0f;
    float stageTime_ = 0.0f;
    float alt_ = 0.0f;
    float vel_ = 0.0f;
    float apogee_ = 0.0f;
};
