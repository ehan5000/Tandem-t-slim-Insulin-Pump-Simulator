#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>

struct ProfileData
{
    double basalRate;         // U/hr
    double carbRatio;         // 1U for X grams of carb
    double correctionFactor;  // 1U lowers BG by X mg/dL
    double targetBG;          // mg/dL
};

class ProfileManager : public QObject
{
    Q_OBJECT
public:
    explicit ProfileManager(QObject *parent = nullptr);

    bool createProfile(const QString &name,
                       double basalRate,
                       double carbRatio,
                       double correctionFactor,
                       double targetBG);

    bool updateProfile(const QString &name,
                       double basalRate,
                       double carbRatio,
                       double correctionFactor,
                       double targetBG);

    bool deleteProfile(const QString &name);

    bool hasProfile(const QString &name) const;
    ProfileData profile(const QString &name) const;
    QStringList profileNames() const;

private:
    QMap<QString, ProfileData> m_profiles;
};

#endif // PROFILEMANAGER_H
