#ifndef SYSTEMLOG_H
#define SYSTEMLOG_H

#include <QObject>
#include <QString>
#include <QStringList>

class SystemLog : public QObject
{
    Q_OBJECT
public:
    explicit SystemLog(QObject *parent = nullptr);
    void addLogEntry(const QString &entry);
    QString fullLog() const;

private:
    QStringList m_entries;
};

#endif // SYSTEMLOG_H
