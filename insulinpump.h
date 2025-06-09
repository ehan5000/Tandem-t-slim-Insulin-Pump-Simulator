#ifndef INSULINPUMP_H
#define INSULINPUMP_H

#include <QObject>
#include "profilemanager.h"
#include "cgm.h"
#include "timesimulator.h"

class InsulinPump : public QObject
{
    Q_OBJECT
public:
    explicit InsulinPump(QObject *parent = nullptr);

    // Profile selection
    void setActiveProfile(const ProfileData &profile);

    // Basal delivery
    void startBasalDelivery();
    void stopBasalDelivery();
    bool isBasalActive() const;
    void setCGM(CGM *cgm);

    // Bolus
    double calculateBolus(double currentBG, double carbIntake);
    bool deliverBolus(double units);

    // Battery & insulin management
    void useBattery(double amount);
    double batteryLevel() const;
    double insulinUnitsRemaining() const;
    void rechargeBattery();
    void replenishInsulin();

    // Simulation tick
    void performBasalTick();

    // time simulator
    void setTimeSimulator(TimeSimulator *sim);

private:
    double m_battery;           // [0..100%]
    double m_insulinRemaining;  // [0..300 units]
    bool   m_basalActive;
    ProfileData m_activeProfile;
    CGM *m_cgm = nullptr;

    //
    TimeSimulator *m_timeSimulator = nullptr;

    // Helper
    void consumeInsulin(double units);
};

#endif // INSULINPUMP_H
