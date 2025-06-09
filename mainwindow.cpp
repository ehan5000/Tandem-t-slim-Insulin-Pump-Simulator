#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QDialog>
#include <QDateTime>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_profileManager(new ProfileManager(this)),
      m_insulinPump(new InsulinPump(this)),
      m_cgm(new CGM(this)),
      m_systemLog(new SystemLog(this)),
      m_timeSimulator(new TimeSimulator(this)),
      m_extBolusRemaining(0.0),
      m_extBolusRatePerTick(0.0),
      m_simulationTimer(new QTimer(this))
{

     m_insulinPump->setTimeSimulator(m_timeSimulator);

    setupUI();

    // Connect CGM alerts
    connect(m_cgm, &CGM::criticalLowGlucose, this, &MainWindow::onCriticalLowGlucose);
    connect(m_cgm, &CGM::criticalHighGlucose, this, &MainWindow::onCriticalHighGlucose);

    // Connect UI actions
    connect(m_createProfileBtn, &QPushButton::clicked, this, &MainWindow::onCreateProfile);
    connect(m_updateProfileBtn, &QPushButton::clicked, this, &MainWindow::onUpdateProfile);
    connect(m_deleteProfileBtn, &QPushButton::clicked, this, &MainWindow::onDeleteProfile);
    connect(m_startInsulinBtn,  &QPushButton::clicked, this, &MainWindow::onStartInsulin);
    connect(m_stopInsulinBtn,   &QPushButton::clicked, this, &MainWindow::onStopInsulin);
    connect(m_manualBolusBtn,   &QPushButton::clicked, this, &MainWindow::onManualBolus);
    connect(m_viewHistoryBtn,   &QPushButton::clicked, this, &MainWindow::onViewHistory);
    connect(m_toggleSimTimeBtn, &QPushButton::clicked, this, &MainWindow::onTimeSimulationToggle);
    connect(m_graph1hBtn,       &QPushButton::clicked, this, &MainWindow::onGraph1h);
    connect(m_graph3hBtn,       &QPushButton::clicked, this, &MainWindow::onGraph3h);
    connect(m_graph6hBtn,       &QPushButton::clicked, this, &MainWindow::onGraph6h);

    // Simulation timer
    connect(m_simulationTimer, &QTimer::timeout, this, &MainWindow::onSimulationTick);
    m_simulationTimer->start(1000);

    // Pre-seed 2h CGM data
    QDateTime now = QDateTime::currentDateTime();
    for(int i = 0; i < 24; ++i)  {
        m_cgm->generateReading(now.addSecs(i * 300));
    }
    // Start time simulator
    m_timeSimulator->start();
    onSimulationTick();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    setWindowTitle("Tandem t:slim X2 Simulator");
    resize(900, 600);

    // Buttons
    m_createProfileBtn  = new QPushButton("Create Profile", this);
    m_updateProfileBtn  = new QPushButton("Update Profile", this);
    m_deleteProfileBtn  = new QPushButton("Delete Profile", this);
    m_startInsulinBtn   = new QPushButton("Start Insulin", this);
    m_stopInsulinBtn    = new QPushButton("Stop Insulin", this);
    m_manualBolusBtn    = new QPushButton("Manual Bolus", this);
    m_viewHistoryBtn    = new QPushButton("View History", this);
    m_toggleSimTimeBtn  = new QPushButton("Pause Simulation", this);
    m_graph1hBtn        = new QPushButton("Graph 1h", this);
    m_graph3hBtn        = new QPushButton("Graph 3h", this);
    m_graph6hBtn        = new QPushButton("Graph 6h", this);

    // Labels
    m_simulatedTimeLabel = new QLabel("Simulated Time: Ready", this);
    m_batteryLabel       = new QLabel("Battery: 100%", this);
    m_insulinLabel       = new QLabel("Insulin: 300U / 300U", this);
    m_statusLabel        = new QLabel("Status: Ready", this);

    // Log viewer
    m_logViewer = new QTextEdit(this);
    m_logViewer->setReadOnly(true);

    // Layout
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    QHBoxLayout *topLayout  = new QHBoxLayout();
    topLayout->addWidget(m_createProfileBtn);
    topLayout->addWidget(m_updateProfileBtn);
    topLayout->addWidget(m_deleteProfileBtn);
    topLayout->addWidget(m_startInsulinBtn);
    topLayout->addWidget(m_stopInsulinBtn);
    topLayout->addWidget(m_manualBolusBtn);
    topLayout->addWidget(m_viewHistoryBtn);
    topLayout->addWidget(m_toggleSimTimeBtn);
    topLayout->addWidget(m_graph1hBtn);
    topLayout->addWidget(m_graph3hBtn);
    topLayout->addWidget(m_graph6hBtn);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_simulatedTimeLabel);
    mainLayout->addWidget(m_batteryLabel);
    mainLayout->addWidget(m_insulinLabel);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_logViewer);
    setCentralWidget(central);
}

// --- User Action Slots ---

void MainWindow::onCreateProfile()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Create Profile", "Profile Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    double br = QInputDialog::getDouble(this, "Basal Rate", "Basal Rate (Units/hour):", 1.0, 0.0, 10.0, 1, &ok);
    if (!ok) return;

    double cr = QInputDialog::getDouble(this, "Carbohydrate Ratio", "1 Unit per X grams of carbs:", 10.0, 1.0, 100.0, 1, &ok);
    if (!ok) return;

    double cf = QInputDialog::getDouble(this, "Correction Factor", "1 Unit lowers BG by X mmol/L:", 2.0, 0.1, 10.0, 1, &ok);
    if (!ok) return;

    double tg = QInputDialog::getDouble(this, "Target BG", "Target Blood Glucose (mmol/L):", 5.5, 3.0, 15.0, 1, &ok);
    if (!ok) return;

    if (!m_profileManager->createProfile(name, br, cr, cf, tg)) {
        QMessageBox::warning(this, "Profile Error", "Profile exists or invalid data");
        return;
    }

    m_currentProfile = {br, cr, cf, tg};
    logEvent(QString("Profile '%1' created").arg(name));
}

void MainWindow::onUpdateProfile()
{
    QStringList names = m_profileManager->profileNames();
    if (names.isEmpty()) {
        QMessageBox::warning(this, "No Profiles", "Create a profile first.");
        return;
    }

    bool ok;
    QString sel = QInputDialog::getItem(this, "Select Profile to Update", "Profiles:", names, 0, false, &ok);
    if (!ok || sel.isEmpty()) return;

    double br = QInputDialog::getDouble(this, "Basal Rate", "Basal Rate (Units/hour):", m_currentProfile.basalRate, 0.0, 10.0, 1, &ok);
    if (!ok) return;

    double cr = QInputDialog::getDouble(this, "Carbohydrate Ratio", "1 Unit per X grams of carbs:", m_currentProfile.carbRatio, 1.0, 100.0, 1, &ok);
    if (!ok) return;

    double cf = QInputDialog::getDouble(this, "Correction Factor", "1 Unit lowers BG by X mmol/L:", m_currentProfile.correctionFactor, 0.1, 10.0, 1, &ok);
    if (!ok) return;

    double tg = QInputDialog::getDouble(this, "Target BG", "Target Blood Glucose (mmol/L):", m_currentProfile.targetBG, 3.0, 15.0, 1, &ok);
    if (!ok) return;

    if (!m_profileManager->updateProfile(sel, br, cr, cf, tg)) {
        QMessageBox::warning(this, "Update Failed", "Could not update profile.");
    } else {
        //Update active profile after edit
        ProfileData updated = m_profileManager->profile(sel);
        m_currentProfile = updated;
        m_insulinPump->setActiveProfile(updated);
        if(!m_userSuspendedInsulin){
            m_statusLabel->setText(QString("Basal active: %1 U/hr").arg(updated.basalRate));
        }
        logEvent(QString("Profile updated: %1").arg(sel));
    }
}

void MainWindow::onDeleteProfile()
{
    QStringList names = m_profileManager->profileNames();
    if (names.isEmpty()) {
        QMessageBox::warning(this, "No Profiles", "Nothing to delete.");
        return;
    }
    bool ok;
    QString sel = QInputDialog::getItem(this, "Select Profile to Delete", "Profiles:", names, 0, false, &ok);
    if (!ok || sel.isEmpty()) return;

    if (QMessageBox::question(this, "Confirm Delete",
        QString("Delete profile '%1'? This cannot be undone.").arg(sel),
        QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    if (!m_profileManager->deleteProfile(sel)) {
        QMessageBox::warning(this, "Delete Failed", "Could not delete profile.");
    } else {
        // If we deleted the active profile, stop basal
        m_insulinPump->stopBasalDelivery();
        m_statusLabel->setText("Basal stopped");
        logEvent(QString("Profile deleted: %1").arg(sel));
    }
}

void MainWindow::onStartInsulin()
{
    QStringList names = m_profileManager->profileNames();
    if (names.isEmpty()) { QMessageBox::warning(this, "No Profiles", "Create a profile first"); return; }
    bool ok;
    QString sel = QInputDialog::getItem(this, "Select Profile", "Profiles:", names, 0, false, &ok);
    if (!ok || sel.isEmpty()) return;
    ProfileData pd = m_profileManager->profile(sel);
    m_currentProfile = pd;
    m_userSuspendedInsulin = false;
    m_insulinPump->setActiveProfile(pd);
    m_insulinPump->startBasalDelivery();
    m_statusLabel->setText(QString("Basal active: %1 U/hr").arg(pd.basalRate));
    logEvent(QString("Basal started with '%1'").arg(sel));
}

void MainWindow::onStopInsulin()
{
    if(!m_userSuspendedInsulin){
        m_userSuspendedInsulin = true;
        m_insulinPump->stopBasalDelivery();
        m_statusLabel->setText("Basal stopped");
        logEvent("Basal stopped");
    }
}

void MainWindow::onManualBolus()
{
    bool ok;

    // 1) Prompt the user, defaulting to the CGM‐derived value
    double bg = QInputDialog::getDouble(
        this,
        "Current BG",
        "mmol/L:",
        m_cgm->currentGlucose(),      // default is your CGM reading
        0.0,          // min
        1000.0,       // max
        1,            // decimals
        &ok
    );
    if (!ok) return;

    // 3) Rest of your bolus logic unchanged
    double carbs = QInputDialog::getDouble(this, "Carbs", "grams:", 0.0, 0.0, 200.0, 1, &ok);
    if (!ok) return;

    double totalBolus = m_insulinPump->calculateBolus(bg, carbs);

    // Immediate fraction prompt
    int frac = QInputDialog::getInt(this, "Immediate Fraction", "% immediate (rest ext):", 60, 0, 100, 1, &ok);
    if (!ok) return;

    // Extended duration prompt
    int hours = QInputDialog::getInt(this, "Extended Duration", "Hours:", 3, 1, 12, 1, &ok);
    if (!ok) return;

    double imm = totalBolus * frac / 100.0;
    double ext = totalBolus - imm;

    // Deliver immediate
    if (m_insulinPump->deliverBolus(imm))
        logEvent(QString("Immediate bolus: %1 U").arg(imm));

    // Schedule extended
    double ticksPerHour = 60.0 / m_timeSimulator->simulationSpeed();
    double totalTicks = ticksPerHour * hours;
    m_extBolusRatePerTick = (totalTicks>0 ? ext / totalTicks : 0.0);
    m_extBolusRemaining = ext;
    if (ext > 0)
        logEvent(QString("Scheduled extended bolus: %1 U over %2 h (%3 U/tick)")
                 .arg(ext).arg(hours).arg(m_extBolusRatePerTick));
}


void MainWindow::onViewHistory()
{
    QString h = m_systemLog->fullLog();
    if (h.isEmpty()) QMessageBox::information(this, "History", "No logs available.");
    else QMessageBox::information(this, "History", h);
}

void MainWindow::onCheckForErrors()
{
    if (m_insulinPump->batteryLevel() < 5.0) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Low Battery");
        msgBox.setText("Battery is critically low.");
        QPushButton *rechargeBtn = msgBox.addButton("Recharge", QMessageBox::AcceptRole);
        msgBox.exec();

        if (msgBox.clickedButton() == rechargeBtn) {
            m_insulinPump->rechargeBattery();
            logEvent("Pump charged to 100%.");
        }
    }

    if (m_insulinPump->insulinUnitsRemaining() < 5.0) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Low Insulin");
        msgBox.setText("Insulin is critically low.");
        QPushButton *replaceBtn = msgBox.addButton("Replace Cartridge", QMessageBox::AcceptRole);
        msgBox.exec();

        if (msgBox.clickedButton() == replaceBtn) {
            m_insulinPump->replenishInsulin();
            logEvent("Pump insulin replenished to 300u.");
        }
    }
}

// --- Simulation Slot ---

void MainWindow::onTimeSimulationToggle()
{
    if (m_timeSimulator->isRunning()) {
        // Stop simulation components
        m_timeSimulator->stop();
        m_simulationTimer->stop();  // <-- stop the periodic update timer too
        m_toggleSimTimeBtn->setText("Start");
        logEvent("Simulation paused");
    } else {
        // Resume simulation
        m_timeSimulator->start();
        m_simulationTimer->start(1000);  // <-- resume updates every 1s
        m_toggleSimTimeBtn->setText("Pause");
        logEvent("Simulation resumed");
    }
}

void MainWindow::onSimulationTick()
{
    // 1) Simulated time label
    m_simulatedTimeLabel->setText("Sim Time: " +
    m_timeSimulator->currentSimulatedTime().toString("hh:mm:ss"));

    // 2) CGM reading
    m_cgm->setBasalActive(m_insulinPump->isBasalActive());
    m_cgm->generateReading(m_timeSimulator->currentSimulatedTime());
    double currentBG = m_cgm->currentGlucose(); //Gets current Blood Glucose Lvl

    // 3) Extended bolus
    if (m_extBolusRemaining > 0.0) {
        double deliver = qMin(m_extBolusRatePerTick, m_extBolusRemaining);
        if (m_insulinPump->deliverBolus(deliver)) {
            m_extBolusRemaining -= deliver;
            if (m_extBolusRemaining <= 0.0) {
                logEvent("Extended bolus completed.");
            }
        }
    }

    // 4) Control-IQ
    // Allow correction bolus regardless of user insulin pause flag
    if (currentBG > HIGH_GLUCOSE_THRESHOLD) {
        double corr = (currentBG - m_currentProfile.targetBG) / m_currentProfile.correctionFactor;
        if (corr > 0 && m_insulinPump->deliverBolus(corr)) {
            logEvent(QString("Control-IQ: Correction bolus %1 U").arg(corr));
        }
    }

    // Basal control — only act if user hasn't explicitly paused
    if (!m_userSuspendedInsulin) {
        // Suspend basal if BG is too low
        if (currentBG < LOW_GLUCOSE_THRESHOLD) {
            if (m_insulinPump->isBasalActive()) {
                m_insulinPump->stopBasalDelivery();
                logEvent("Control-IQ: Basal suspended (low BG)");
            }
        } else {
            // Resume basal if BG is safe
            if (!m_insulinPump->isBasalActive() && !m_profileManager->profileNames().isEmpty()) {
                m_insulinPump->startBasalDelivery();
                logEvent("Control-IQ: Basal resumed (safe BG)");
            }
        }
    }

    // 5) Basal tick + battery
    m_insulinPump->performBasalTick();
    m_insulinPump->useBattery(0.01);

    // 6) Update labels
    m_batteryLabel->setText(QString("Battery: %1% ")
        .arg(m_insulinPump->batteryLevel(), 0, 'f', 1));
    m_insulinLabel->setText(QString("Insulin: %1U/300U")
        .arg(m_insulinPump->insulinUnitsRemaining(), 0, 'f', 1));

    // 7) Error checks
    onCheckForErrors();
}

// --- Graph Slots ---

void MainWindow::onGraph1h() { plotGlucoseGraph(1); }
void MainWindow::onGraph3h() { plotGlucoseGraph(3); }
void MainWindow::onGraph6h() { plotGlucoseGraph(6); }

// --- CGM Alerts ---

void MainWindow::onCriticalLowGlucose(double value)
{
    //QMessageBox::warning(this, "Low Glucose",
                         //QString("Critical low glucose: %1 mmol/L").arg(value));
    logEvent(QString("Critical low CGM alert: %1").arg(value));
}

void MainWindow::onCriticalHighGlucose(double value)
{
    //QMessageBox::warning(this, "High Glucose",
                         //QString("Critical high glucose: %1 mmol/L").arg(value));
    logEvent(QString("Critical high CGM alert: %1").arg(value));
}

// --- Graph Helper ---

void MainWindow::plotGlucoseGraph(int hours)
{
    auto readings = m_cgm->getReadings(hours);
    if (readings.isEmpty()) {
        QMessageBox::information(this, "No Data", "Not enough CGM data for that period.");
        return;
    }
    QLineSeries *series = new QLineSeries();
    for (const auto &r : readings) {
        series->append(r.timestamp.toMSecsSinceEpoch(), r.value);
    }
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();
    chart->setTitle(QString("Glucose Trend (%1h)").arg(hours));
    QDateTimeAxis *axisX = new QDateTimeAxis(); axisX->setFormat("hh:mm");
    axisX->setTickCount(6);
    axisX->setLabelsAngle(-45);
    axisX->setTitleText("Time"); axisX->setMin(readings.front().timestamp);
    axisX->setMax(readings.back().timestamp); chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    QValueAxis *axisY = new QValueAxis(); axisY->setLabelFormat("%.1f");
    axisY->setTitleText("Glucose (mmol/L)"); chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    QChartView *view = new QChartView(chart); view->setRenderHint(QPainter::Antialiasing);
    QDialog dlg(this); dlg.setWindowTitle("Glucose Graph");
    QVBoxLayout *layout = new QVBoxLayout(&dlg); layout->addWidget(view);
    dlg.resize(700, 400); dlg.exec();
}

// --- Helper ---

void MainWindow::logEvent(const QString &msg)
{
    // Use simulated time for log timestamps
    QString simTs = m_timeSimulator->currentSimulatedTime().toString("yyyy-MM-dd hh:mm:ss");
    QString entry = QString("[%1] %2").arg(simTs, msg);

    // Store in system log and update text viewer
    m_systemLog->addLogEntry(entry);
    m_logViewer->append(entry);
}
