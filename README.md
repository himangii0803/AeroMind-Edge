# ✈️ AeroMind Edge

An Edge AI–inspired predictive maintenance dashboard for aircraft engines built using the NASA C-MAPSS dataset.

AeroMind Edge analyzes engine health, estimates Remaining Useful Life (RUL), classifies maintenance risk, and generates maintenance recommendations through an interactive dashboard.

---

# Features

- Aircraft fleet monitoring dashboard
- Remaining Useful Life (RUL) estimation
- Engine health score calculation
- Failure risk classification
- Maintenance Copilot with recommendations
- Fleet analytics dashboard
- Risk distribution charts
- Remaining Useful Life visualization
- Engine search and filtering
- Fleet and engine report generation
- Dark mode support
- Built using the NASA C-MAPSS dataset

---

# Project Architecture

```
NASA C-MAPSS Dataset
        │
        ▼
C++ Prediction Engine
        │
Engine Health Analysis
        │
Remaining Useful Life Prediction
        │
JSON Generation
        │
Interactive Dashboard
        │
Maintenance Copilot
```

---

# Folder Structure

```
AeroMind-Edge
│
├── backend
│   ├── main.cpp
│   ├── predictor.cpp
│   └── predictor.h
│
├── frontend
│   ├── index.html
│   ├── style.css
│   ├── script.js
│   └── data.json
│
├── data
│   ├── train
│   ├── test
│   ├── rul
│   └── processed
│
└── README.md
```

---

# Tech Stack

### Backend

- C++
- STL
- File Handling
- JSON Generation

### Frontend

- HTML5
- CSS3
- JavaScript
- Chart.js

### Dataset

NASA C-MAPSS Turbofan Engine Degradation Dataset

---

# Dashboard Modules

## Fleet Overview

Displays:

- Total Engines
- Average Fleet Health
- Average Remaining Useful Life
- Critical Engines

---

## Fleet Analytics

- Fleet Risk Distribution
- Remaining Useful Life Distribution
- Engine Risk Classification

---

## Maintenance Copilot

Provides

- Health Score
- Failure Risk
- Remaining Useful Life
- Detected Condition
- Maintenance Insight
- Inspection Checklist
- Recommended Action

---

# Risk Classification

| Health Score | Risk |
|--------------|------|
| High | Low |
| Medium | Moderate |
| Low | Critical |

---

# Dataset

The project uses the NASA C-MAPSS aircraft engine degradation dataset for predictive maintenance analysis.

The dataset contains

- Multiple aircraft engines
- Engine operating cycles
- Operating settings
- Sensor measurements
- Remaining Useful Life information

---

# How to Run

## Compile

```bash
g++ -std=c++17 backend/main.cpp backend/predictor.cpp -o aeromind
```

## Run Backend

```bash
./aeromind
```

This generates

```
frontend/data.json
```

## Launch Dashboard

```bash
cd frontend
python3 -m http.server 8000
```

Open

```
http://localhost:8000
```

---

# Future Improvements

- Sensor trend visualization
- Predictive machine learning models
- Fleet maintenance scheduling
- Aircraft digital twin integration
- Cloud synchronization
- Multi-fleet monitoring
- Predictive alert notifications

---

# Project Highlights

- Real NASA dataset
- C++ backend
- Interactive dashboard
- Predictive maintenance workflow
- Maintenance Copilot
- Fleet analytics
- Report generation
- Responsive UI
- Dark mode

---

# License

This project is developed for educational, research, and hackathon purposes.
