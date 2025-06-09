// Fixed TimeSimulator: Starts at 00:00:00, adds 5 simulated minutes per real second
#include "timesimulator.h"

TimeSimulator::TimeSimulator(QObject *parent)
   : QObject(parent),
     m_minutesPerSecond(SIMULATION_SPEED),
     m_running(false),
     m_totalSimulatedMinutes(0.0)
{
   m_timer.setInterval(1000);  // 1-second real time interval
   connect(&m_timer, &QTimer::timeout, this, &TimeSimulator::onTimerTick);
}

double TimeSimulator::simulationSpeed() const
{
   return m_minutesPerSecond;
}

QDateTime TimeSimulator::currentSimulatedTime() const
{
   // Always start from a zero time and add total simulated minutes
   QDateTime base(QDate(2025, 1, 1), QTime(0, 0, 0));
   return base.addSecs(static_cast<qint64>(m_totalSimulatedMinutes * 60));
}

void TimeSimulator::start()
{
   if (!m_running) {
       m_running = true;
       m_timer.start();
   }
}

void TimeSimulator::stop()
{
   if (m_running) {
       m_running = false;
       m_timer.stop();
   }
}

bool TimeSimulator::isRunning() const
{
   return m_running;
}

void TimeSimulator::reset()
{
   stop();
   m_totalSimulatedMinutes = 0.0;
}

double TimeSimulator::realToSimulatedSeconds(double realSeconds) const
{
   return realSeconds * m_minutesPerSecond * 60;
}

double TimeSimulator::realToSimulatedMinutes(double realSeconds) const
{
   return realSeconds * m_minutesPerSecond;
}

void TimeSimulator::onTimerTick()
{
   // Add simulated minutes per tick
   m_totalSimulatedMinutes += m_minutesPerSecond;
   emit simulationTick(m_minutesPerSecond);
}
