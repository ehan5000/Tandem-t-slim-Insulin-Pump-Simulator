#include "systemlog.h"
#include <QDateTime>

SystemLog::SystemLog(QObject *parent)
    : QObject(parent)
{
}

void SystemLog::addLogEntry(const QString &entry)
{
    // entry already contains the simulated‐time timestamp
    m_entries.append(entry);
}


QString SystemLog::fullLog() const
{
    return m_entries.join("\n");
}
