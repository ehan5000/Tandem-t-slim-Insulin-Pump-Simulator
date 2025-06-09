// timesimulator.h
#ifndef TIMESIMULATOR_H
#define TIMESIMULATOR_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
//Constant for how many minutes pass in simulation per tick
const double SIMULATION_SPEED = 5.0;

class TimeSimulator : public QObject
{
    Q_OBJECT
public:
    explicit TimeSimulator(QObject *parent = nullptr);

    // Configure simulation speed (simulatedMinutes per real second)
    void setSimulationSpeed(double minutesPerSecond);
    double simulationSpeed() const;

    // Get current simulated time
    QDateTime currentSimulatedTime() const;

    // Start/stop time simulation
    void start();
    void stop();
    bool isRunning() const;

    // Reset simulation time to current real time
    void reset();

    // Convert real time delta to simulated time delta
    double realToSimulatedSeconds(double realSeconds) const;
    double realToSimulatedMinutes(double realSeconds) const;

signals:
    // Emitted at each simulation tick
    void simulationTick(double elapsedSimulatedMinutes);

private slots:
    void onTimerTick();

private:
    QTimer m_timer;                // Timer for regular updates
    QDateTime m_simulationStart;   // Real time when simulation started
    QDateTime m_simulatedStart;    // Initial simulated time
    double m_minutesPerSecond;     // Simulation speed
    bool m_running;                // Is simulation running?
    double m_totalSimulatedMinutes; // Total simulated minutes since start
};

#endif // TIMESIMULATOR_H
