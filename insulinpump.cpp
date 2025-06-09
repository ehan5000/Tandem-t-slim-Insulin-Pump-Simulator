#include "insulinpump.h"
#include <QDebug>

InsulinPump::InsulinPump(QObject *parent)
    : QObject(parent),
      m_battery(100.0),
      m_insulinRemaining(300.0),
      m_basalActive(false)
{
}

void InsulinPump::setActiveProfile(const ProfileData &profile)
{
    m_activeProfile = profile;
}

void InsulinPump::startBasalDelivery()
{
    m_basalActive = true;
}

void InsulinPump::stopBasalDelivery()
{
    m_basalActive = false;
}

bool InsulinPump::isBasalActive() const
{
    return m_basalActive;
}

void InsulinPump::setCGM(CGM *cgm) {
    m_cgm = cgm;
}

void InsulinPump::setTimeSimulator(TimeSimulator *sim) {
    m_timeSimulator = sim;
}

double InsulinPump::calculateBolus(double currentBG, double carbIntake)
{

    // 1. Carbohydrate coverage: carbs / carbRatio
    // 2. Correction if currentBG > targetBG: (currentBG - targetBG) / correctionFactor
    double insulinForCarbs = carbIntake / m_activeProfile.carbRatio;
    double correction = 0.0;
    if(currentBG > m_activeProfile.targetBG) {
        correction = (currentBG - m_activeProfile.targetBG) / m_activeProfile.correctionFactor;
    }
    return insulinForCarbs + correction;
}

bool InsulinPump::deliverBolus(double units)
{
    // Check if enough insulin is left
    if(m_insulinRemaining < units || units <= 0.0) {
        return false;
    }
    consumeInsulin(units);
    return true;
}

void InsulinPump::useBattery(double amount)
{
    m_battery -= amount;
    if(m_battery < 0) {
        m_battery = 0;
    }
}

double InsulinPump::batteryLevel() const
{
    return m_battery;
}

double InsulinPump::insulinUnitsRemaining() const
{
    return m_insulinRemaining;
}

void InsulinPump::performBasalTick()
{
    if (!m_basalActive || m_activeProfile.basalRate <= 0.0)
        return;

    // 1 unit per hour = basalRate / 60 = units per minute
    double insulinPerMinute = m_activeProfile.basalRate / 60.0;

    // Determine how many minutes each tick represents
    double simMinutesPerTick = SIMULATION_SPEED; // fallback
    if (m_timeSimulator) {
        simMinutesPerTick = m_timeSimulator->simulationSpeed(); // e.g. 5.0 means 5 minutes per tick
    }

    double insulinThisTick = insulinPerMinute * simMinutesPerTick;

    if (m_insulinRemaining < insulinThisTick) {
        insulinThisTick = m_insulinRemaining; // don't go negative
    }

    consumeInsulin(insulinThisTick);

    // Notify CGM of insulin effect
    if (m_cgm && insulinThisTick > 0.0) {
        m_cgm->registerInsulinEffect(insulinThisTick);
    }
}

void InsulinPump::consumeInsulin(double units)
{
    m_insulinRemaining -= units;
    if(m_insulinRemaining < 0) {
        m_insulinRemaining = 0;
    }
}

void InsulinPump::rechargeBattery() {
    m_battery = 100.0;
}

void InsulinPump::replenishInsulin() {
    m_insulinRemaining = 300.0;
}
