// cgm.cpp
#include "cgm.h"

CGM::CGM(QObject *parent)
    : QObject(parent),
      m_baseGlucose(DEFAULT_BASE_GLUCOSE),
      m_pendingInsulinEffect(0.0),
      m_pendingCarbEffect(0.0)
{
    // Initialize with a starting reading
    GlucoseReading initialReading;
    initialReading.timestamp = QDateTime::currentDateTime();
    initialReading.value = m_baseGlucose;
    m_readings.append(initialReading);
}

double CGM::currentGlucose() const
{
    if (m_readings.isEmpty()) {
        return m_baseGlucose;
    }
    return m_readings.last().value;
}

double CGM::glucoseTrend() const
{
    if (m_readings.size() < 2) {
        return 0.0;
    }

    // Calculate the difference between last two readings
    return m_readings.last().value - m_readings[m_readings.size() - 2].value;
}

void CGM::generateReading(const QDateTime &simulatedTime)
{
    GlucoseReading newReading;
    newReading.timestamp = simulatedTime;
    newReading.value = calculateNextGlucose();

    if (isValidReading(newReading.value)) {
        m_readings.append(newReading);

        if (newReading.value <= LOW_GLUCOSE_THRESHOLD) {
            emit criticalLowGlucose(newReading.value);
        } else if (newReading.value >= HIGH_GLUCOSE_THRESHOLD) {
            emit criticalHighGlucose(newReading.value);
        }

        m_pendingInsulinEffect *= 0.95;
        m_pendingCarbEffect *= 0.9;
    }

    if (m_readings.size() > 288) {
        m_readings.removeFirst();
    }
}

QVector<GlucoseReading> CGM::getReadings(int hours) const
{
    // Return readings for the specified number of hours (12 per hour at 5 min intervals)
    int count = std::min(hours * 12, m_readings.size());
    if (count <= 0) {
        return QVector<GlucoseReading>();
    }

    return m_readings.mid(m_readings.size() - count);
}

/*
double CGM::predictGlucose(int minutesInFuture) const
{
    // Simple linear prediction based on recent trend
    if (m_readings.size() < 2) {
        return currentGlucose();
    }

    // Calculate average change per minute over last few readings
    double avgChangePerMinute = 0.0;
    int numReadings = std::min(3, m_readings.size() - 1);

    for (int i = 0; i < numReadings; i++) {
        double diff = m_readings[m_readings.size() - 1 - i].value -
                      m_readings[m_readings.size() - 2 - i].value;

        // Assuming readings are 5 minutes apart
        avgChangePerMinute += diff / 5.0;
    }

    avgChangePerMinute /= numReadings;

    // Predict future glucose by extrapolating the trend
    double futureBG = currentGlucose() + (avgChangePerMinute * minutesInFuture);

    // Account for pending insulin and carb effects
    futureBG -= m_pendingInsulinEffect * (minutesInFuture / 30.0);
    futureBG += m_pendingCarbEffect * (minutesInFuture / 30.0);

    return std::max(0.0, futureBG);
}
*/

void CGM::setBaseGlucose(double baseLevel)
{
    if (baseLevel > 0.0 && baseLevel < MAX_VALID_GLUCOSE) {
        m_baseGlucose = baseLevel;
    }
}

void CGM::registerInsulinEffect(double units)
{
    // Each unit of insulin will lower BG by approximately 1-3 mmol/L or 18-54 mg/dL
    // Effect peaks at around 60-90 minutes and lasts ~3-5 hours
    m_pendingInsulinEffect += units * 1.0; // Simple approximation
}

void CGM::registerCarbEffect(double grams)
{
    // Carbohydrates raise blood glucose
    // Effect typically starts within 15 minutes and peaks at 45-60 minutes
    m_pendingCarbEffect += grams * 0.05; // Simple approximation
}

double CGM::calculateNextGlucose() const
{
    // Start with the current glucose level
    double nextValue = currentGlucose();

    if (m_basalActive) {
        // Add some natural variation/noise
        double noise = QRandomGenerator::global()->generateDouble() * 0.4 - 0.2;
        nextValue += noise;
        // Apply a pull towards the base level (homeostasis simulation)
        double homeostasisEffect = (m_baseGlucose - nextValue) * 0.05;
        nextValue += homeostasisEffect;
    } else {
        nextValue += 0.1; // Basal suspended, linear rise of 0.1 mmol/L
    }

    // Apply pending insulin and carb effects
    nextValue -= m_pendingInsulinEffect * 0.2;
    nextValue += m_pendingCarbEffect * 0.25;

    return nextValue;
}

bool CGM::isValidReading(double value) const
{
    // Ensure glucose value is within a reasonable physiological range
    return (value > 0.0 && value < MAX_VALID_GLUCOSE);
}

void CGM::setBasalActive(bool active) {
    m_basalActive = active;
}
