# COMP-3004 Final Project: t:slim X2 Insulin Pump Simulator

This Qt/C++ application simulates the Tandem t:slim X2 insulin pump. It supports:

- **Profile management** (create, update, delete personal insulin profiles)  
- **Basal delivery** controlled by a simulated clock (1 real s = 5 sim min)  
- **Manual bolus** with immediate + extended delivery, defaulting to CGM readings  
- **Control‑IQ automation** (30 min prediction → suspend/resume basal + correction boluses)  
- **CGM simulation** at 5 min intervals, with 1h/3h/6h graphing via QtCharts  
- **Error handling** (low battery/insulin and hypo/hyperglycemia alerts)  
- **Simulated‑time logs** viewable in‑app  

---

## Prerequisites

- **Qt 5.x** or **Qt 6.x** with **Widgets** and **Charts** modules  
  - Ubuntu:  
    ```bash
    sudo apt-get update
    sudo apt-get install qt5-default libqt5charts5-dev
    ```
- **C++11** compiler

---

## Project Structure

```text
├── main.cpp               # Application entry point
├── mainwindow.h/.cpp      # UI layout, slots, simulation tick logic
├── insulinpump.h/.cpp     # Basal & bolus logic, battery/insulin tracking
├── cgm.h/.cpp             # CGM data generation, critical alerts
├── timesimulator.h/.cpp   # Simulated clock driving ticks
├── profilemanager.h/.cpp  # CRUD for insulin profiles
├── systemlog.h/.cpp       # Accumulates simulated‑time–stamped events
├── tslim-simulator.pro    # Qt project file
└── README.md              # This file
```

---

## Pull latest changes 

- git pull origin main

---

## Make Edits

- git status              # review changes
- git add .               # stage all changes (or list specific files)
- git commit -m "Describe your change here"
- git push origin main


---


## Feature Walk‑Through & Testing

### Profile CRUD
- **Create**: Click **Create Profile**, enter name + settings.  
- **Update**: Click **Update Profile**, select one, change settings.  
- **Delete**: Click **Delete Profile**, confirm, and observe basal stop if active.  
- **Log**: Watch the log pane for each action.

### Normal Basal Delivery
- After creating a profile, click **Start Insulin** and select it.  
- Verify the **Status** label shows “Basal active” and the insulin level decreases.  
- Click **Stop Insulin**—basal should stop and log that event.

### Manual Bolus
- Click **Manual Bolus** (the BG prompt defaults to the latest CGM value).  
- Enter carbs, choose immediate % and extended hours.  
- Observe an immediate log entry and scheduled extended doses over the next ticks.

### Control‑IQ Automation
- Use a profile with a low target (e.g. 70 mg/dL).  
- Let the sim run; in the log you’ll see “Control‑IQ: Basal suspended” when predicted BG dips, and “Control‑IQ: Correction bolus” when it rises above target.

### Error Conditions
- **Low Battery/Insulin**: In code, temporarily set `m_battery = 3.0;` or `m_insulinRemaining = 4.0;` in `InsulinPump`’s constructor to force a warning on the next tick.  
- **Hypo/Hyperglycemia**: Adjust `DEFAULT_BASE_GLUCOSE` in `cgm.cpp` to `3.5` or `15.0` to trigger critical alerts.

### Graphing CGM
- Immediately click **Graph 1h/3h/6h**—you should see a chart (the 2 h pre‑seed guarantees data even for 3 h).  
- Let the sim run another 10–20 s and graph again to see newly added points.

### View History
- Click **View History** at any time to pop up a dialog showing every event with simulated timestamps.  
- Scroll to confirm entries from startup, profile changes, boluses, Control‑IQ actions, error alerts, and CGM alerts.
