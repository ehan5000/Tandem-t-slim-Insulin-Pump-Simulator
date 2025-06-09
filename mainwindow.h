// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>
#include <QMessageBox>
#include "profilemanager.h"
#include "insulinpump.h"
#include "cgm.h"
#include "systemlog.h"
#include "timesimulator.h"
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // User actions
    void onCreateProfile();
    void onUpdateProfile();
    void onDeleteProfile();
    void onStartInsulin();
    void onStopInsulin();
    void onManualBolus();
    void onViewHistory();
    void onCheckForErrors();

    // Simulation controls
    void onSimulationTick();
    void onTimeSimulationToggle();

    // CGM alerts
    void onCriticalLowGlucose(double value);
    void onCriticalHighGlucose(double value);

    // Graphing
    void onGraph1h();
    void onGraph3h();
    void onGraph6h();

private:
    void setupUI();
    void logEvent(const QString &msg);
    void plotGlucoseGraph(int hours);

    // Core objects
    ProfileManager *m_profileManager;
    InsulinPump    *m_insulinPump;
    CGM            *m_cgm;
    SystemLog      *m_systemLog;
    TimeSimulator  *m_timeSimulator;

    // Current profile for Control-IQ
    ProfileData     m_currentProfile;

    // Extended bolus tracking
    double m_extBolusRemaining;
    double m_extBolusRatePerTick;

    //Flag for user suspended basal insulin
    bool m_userSuspendedInsulin = false;

    // UI elements
    QPushButton *m_createProfileBtn;
    QPushButton *m_updateProfileBtn;
    QPushButton *m_deleteProfileBtn;
    QPushButton *m_startInsulinBtn;
    QPushButton *m_stopInsulinBtn;
    QPushButton *m_manualBolusBtn;
    QPushButton *m_viewHistoryBtn;
    QPushButton *m_toggleSimTimeBtn;
    QPushButton *m_graph1hBtn;
    QPushButton *m_graph3hBtn;
    QPushButton *m_graph6hBtn;
    QLabel      *m_simulatedTimeLabel;
    QLabel      *m_batteryLabel;
    QLabel      *m_insulinLabel;
    QLabel      *m_statusLabel;
    QTextEdit   *m_logViewer;

    // Timer
    QTimer      *m_simulationTimer;
};

#endif // MAINWINDOW_H
