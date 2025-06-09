// cgm.h
#ifndef CGM_H
#define CGM_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>
#include <algorithm>

// Constants for glucose simulation
const double LOW_GLUCOSE_THRESHOLD = 3.9;   // mmol/L or 70 mg/dL
const double HIGH_GLUCOSE_THRESHOLD = 10; // mmol/L or 250 mg/dL
const double DEFAULT_BASE_GLUCOSE = 5.6;    // mmol/L or 100 mg/dL
const double MAX_VALID_GLUCOSE = 33.3;      // mmol/L or 600 mg/dL

struct GlucoseReading {
    QDateTime timestamp;
    double value;      // Blood glucose value in mmol/L or mg/dL
};

class CGM : public QObject
{
    Q_OBJECT
public:
    explicit CGM(QObject *parent = nullptr);

    // Get the current glucose reading
    double currentGlucose() const;

    // Get the trend (difference between current and previous reading)
    double glucoseTrend() const;

    // Generate a new reading (called by simulation timer)
    void generateReading(const QDateTime &simulatedTime);

    // Get historical readings for graphing
    QVector<GlucoseReading> getReadings(int hours) const;

    // Predict glucose level at a future time (for Control-IQ) (feature removed)
    //double predictGlucose(int minutesInFuture) const;

    // Set a base glucose level for simulation
    void setBaseGlucose(double baseLevel);

    // Register insulin effect (will lower future readings)
    void registerInsulinEffect(double units);

    // Register carb effect (will raise future readings)
    void registerCarbEffect(double grams);

    // Sets basal activity to True or False
    void setBasalActive(bool active);

signals:
    void criticalLowGlucose(double value);  // Below 3.9 mmol/L (70 mg/dL)
    void criticalHighGlucose(double value); // Above 10 mmol/L (250 mg/dL)

private:
    QVector<GlucoseReading> m_readings;     // Historical readings
    double m_baseGlucose;                   // Base glucose level for simulation
    double m_pendingInsulinEffect;          // How much insulin is affecting glucose
    double m_pendingCarbEffect;             // How much carbs are affecting glucose
    bool m_basalActive = true;              // Boolean tracking basal activity

    // Helper functions
    double calculateNextGlucose() const;
    bool isValidReading(double value) const;
};

#endif // CGM_H
