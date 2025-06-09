#include "profilemanager.h"

ProfileManager::ProfileManager(QObject *parent)
    : QObject(parent)
{
}

bool ProfileManager::createProfile(const QString &name,
                                   double basalRate,
                                   double carbRatio,
                                   double correctionFactor,
                                   double targetBG)
{
    if(name.isEmpty() || m_profiles.contains(name)) {
        return false;  // Already exists or invalid
    }

    ProfileData data;
    data.basalRate       = basalRate;
    data.carbRatio       = carbRatio;
    data.correctionFactor= correctionFactor;
    data.targetBG        = targetBG;

    m_profiles.insert(name, data);
    return true;
}

bool ProfileManager::updateProfile(const QString &name,
                                   double basalRate,
                                   double carbRatio,
                                   double correctionFactor,
                                   double targetBG)
{
    if(!m_profiles.contains(name)) {
        return false;
    }

    ProfileData &data = m_profiles[name];
    data.basalRate        = basalRate;
    data.carbRatio        = carbRatio;
    data.correctionFactor = correctionFactor;
    data.targetBG         = targetBG;
    return true;
}

bool ProfileManager::deleteProfile(const QString &name)
{
    if(!m_profiles.contains(name)) {
        return false;
    }
    m_profiles.remove(name);
    return true;
}

bool ProfileManager::hasProfile(const QString &name) const
{
    return m_profiles.contains(name);
}

ProfileData ProfileManager::profile(const QString &name) const
{
    return m_profiles.value(name);
}

QStringList ProfileManager::profileNames() const
{
    return m_profiles.keys();
}
