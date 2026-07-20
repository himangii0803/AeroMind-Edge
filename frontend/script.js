let engineData = [];
let selectedEngine = null;
let riskChart = null;
let rulChart = null;

const engineTableBody = document.getElementById("engineTableBody");
const searchInput = document.getElementById("searchInput");
const riskFilter = document.getElementById("riskFilter");

const modal = document.getElementById("engineModal");
const closeModalButton = document.getElementById("closeModal");
const modalOverlay = document.getElementById("modalOverlay");

async function loadDashboard() {
    try {
        const response = await fetch("data.json");

        if (!response.ok) {
            throw new Error("Unable to load dashboard data.");
        }

        const data = await response.json();

        engineData = data.engines;

        updateSummary(data);
        renderCharts(data);
        updatePriorityPanel(data);
        renderTable(engineData);
    } catch (error) {
        engineTableBody.innerHTML = `
            <tr>
                <td colspan="7">
                    Dashboard data could not be loaded. Run the C++ backend first.
                </td>
            </tr>
        `;

        console.error(error);
    }
}

function updateSummary(data) {
    const totalRul = engineData.reduce(
        (sum, engine) => sum + engine.predictedRul,
        0
    );

    const averageRul = engineData.length
        ? Math.round(totalRul / engineData.length)
        : 0;

    document.getElementById("totalEngines").textContent =
        data.summary.totalEngines;

    document.getElementById("averageHealth").textContent =
        `${data.summary.averageHealth}%`;

    document.getElementById("averageRul").textContent =
        averageRul;

    document.getElementById("criticalRisk").textContent =
        data.summary.criticalRisk;
}

function renderCharts(data) {
    const chartTextColor = getComputedStyle(document.body)
        .getPropertyValue("--muted")
        .trim();

    const gridColor = getComputedStyle(document.body)
        .getPropertyValue("--border")
        .trim();

    if (riskChart) {
        riskChart.destroy();
    }

    if (rulChart) {
        rulChart.destroy();
    }

    const riskContext = document
        .getElementById("riskChart")
        .getContext("2d");

    riskChart = new Chart(riskContext, {
        type: "doughnut",
        data: {
            labels: ["Low", "Moderate", "Critical"],
            datasets: [
                {
                    data: [
                        data.summary.lowRisk,
                        data.summary.moderateRisk,
                        data.summary.criticalRisk
                    ],
                    backgroundColor: [
                        "#22c55e",
                        "#f59e0b",
                        "#ef4444"
                    ],
                    borderWidth: 0
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            cutout: "67%",
            plugins: {
                legend: {
                    position: "bottom",
                    labels: {
                        color: chartTextColor,
                        usePointStyle: true,
                        padding: 18
                    }
                }
            }
        }
    });

    const rulGroups = {
        "0–30": 0,
        "31–60": 0,
        "61–90": 0,
        "91–120": 0,
        "120+": 0
    };

    engineData.forEach((engine) => {
        const rul = engine.predictedRul;

        if (rul <= 30) {
            rulGroups["0–30"]++;
        } else if (rul <= 60) {
            rulGroups["31–60"]++;
        } else if (rul <= 90) {
            rulGroups["61–90"]++;
        } else if (rul <= 120) {
            rulGroups["91–120"]++;
        } else {
            rulGroups["120+"]++;
        }
    });

    const rulContext = document
        .getElementById("rulChart")
        .getContext("2d");

    rulChart = new Chart(rulContext, {
        type: "bar",
        data: {
            labels: Object.keys(rulGroups),
            datasets: [
                {
                    label: "Number of Engines",
                    data: Object.values(rulGroups),
                    backgroundColor: "#0795d2",
                    borderRadius: 7
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    display: false
                }
            },
            scales: {
                x: {
                    ticks: {
                        color: chartTextColor
                    },
                    grid: {
                        display: false
                    }
                },
                y: {
                    beginAtZero: true,
                    ticks: {
                        color: chartTextColor,
                        precision: 0
                    },
                    grid: {
                        color: gridColor
                    }
                }
            }
        }
    });
}

function updatePriorityPanel(data) {
    const description = document.getElementById("priorityDescription");

    if (data.summary.criticalRisk > 0) {
        description.textContent =
            `${data.summary.criticalRisk} engines are classified as critical and require priority inspection.`;
    } else {
        description.textContent =
            "No engines currently require immediate maintenance action.";
    }
}

function getRiskClass(riskLevel) {
    if (riskLevel === "LOW") {
        return "risk-low";
    }

    if (riskLevel === "MODERATE") {
        return "risk-moderate";
    }

    return "risk-critical";
}

function renderTable(engines) {
    engineTableBody.innerHTML = "";

    if (engines.length === 0) {
        engineTableBody.innerHTML = `
            <tr>
                <td colspan="7">
                    No engines match the selected filters.
                </td>
            </tr>
        `;

        return;
    }

    engines.forEach((engine) => {
        const row = document.createElement("tr");

        row.innerHTML = `
            <td><strong>${engine.engineId}</strong></td>
            <td>${engine.currentCycle}</td>
            <td>${engine.predictedRul} cycles</td>

            <td>
                <div>${engine.healthScore}%</div>

                <div class="health-bar">
                    <div
                        class="health-bar-fill"
                        style="width: ${engine.healthScore}%"
                    ></div>
                </div>
            </td>

            <td>${engine.failureRisk}%</td>

            <td>
                <span class="risk-badge ${getRiskClass(engine.riskLevel)}">
                    ${engine.riskLevel}
                </span>
            </td>

            <td>
                <button
                    class="details-button"
                    data-engine-id="${engine.engineId}"
                >
                    Launch Copilot
                </button>
            </td>
        `;

        engineTableBody.appendChild(row);
    });

    document.querySelectorAll(".details-button").forEach((button) => {
        button.addEventListener("click", () => {
            openEngineModal(button.dataset.engineId);
        });
    });
}

function filterEngines() {
    const searchValue = searchInput.value.trim().toLowerCase();
    const selectedRisk = riskFilter.value;

    const filteredEngines = engineData.filter((engine) => {
        const matchesSearch =
            engine.engineId.toLowerCase().includes(searchValue);

        const matchesRisk =
            selectedRisk === "ALL" ||
            engine.riskLevel === selectedRisk;

        return matchesSearch && matchesRisk;
    });

    renderTable(filteredEngines);
}

function calculateConfidence(engine) {
    const baseConfidence = 82;
    const riskAdjustment = Math.round(engine.failureRisk * 0.12);
    const cycleAdjustment = Math.min(
        6,
        Math.round(engine.currentCycle / 50)
    );

    return Math.min(
        98,
        baseConfidence + riskAdjustment + cycleAdjustment
    );
}

function getMaintenanceTime(engine) {
    if (engine.riskLevel === "CRITICAL") {
        return "Estimated 5–8 hours";
    }

    if (engine.riskLevel === "MODERATE") {
        return "Estimated 2–4 hours";
    }

    return "Routine inspection under 2 hours";
}

function getChecklist(engine) {
    if (engine.riskLevel === "CRITICAL") {
        return [
            "Inspect compressor and turbine assemblies",
            "Check lubrication pressure and oil contamination",
            "Review recent temperature and pressure sensor trends",
            "Perform vibration and bearing inspection",
            "Confirm engine clearance before return to service"
        ];
    }

    if (engine.riskLevel === "MODERATE") {
        return [
            "Review recent sensor trend changes",
            "Inspect lubrication and pressure systems",
            "Perform targeted compressor inspection",
            "Increase monitoring during the next operating cycles"
        ];
    }

    return [
        "Continue standard condition monitoring",
        "Verify sensor calibration during scheduled maintenance",
        "Follow the preventive maintenance schedule"
    ];
}

function openEngineModal(engineId) {
    const engine = engineData.find(
        (item) => item.engineId === engineId
    );

    if (!engine) {
        return;
    }

    selectedEngine = engine;

    const confidence = calculateConfidence(engine);
    const checklist = getChecklist(engine);

    document.getElementById("modalEngineId").textContent =
        engine.engineId;

    document.getElementById("modalHealth").textContent =
        `${engine.healthScore}%`;

    document.getElementById("modalRisk").textContent =
        `${engine.failureRisk}%`;

    document.getElementById("modalRul").textContent =
        `${engine.predictedRul} cycles`;

    document.getElementById("modalConfidence").textContent =
        `${confidence}%`;

    document.getElementById("modalIssue").textContent =
        engine.detectedIssue;

    document.getElementById("modalMaintenanceTime").textContent =
        getMaintenanceTime(engine);

    document.getElementById("modalExplanation").textContent =
        engine.explanation;

    document.getElementById("modalRecommendation").textContent =
        engine.recommendation;

    const riskBadge = document.getElementById("modalRiskBadge");

    riskBadge.textContent = engine.riskLevel;
    riskBadge.className =
        `modal-risk-badge ${getRiskClass(engine.riskLevel)}`;

    const checklistElement =
        document.getElementById("modalChecklist");

    checklistElement.innerHTML = "";

    checklist.forEach((item) => {
        const listItem = document.createElement("li");
        listItem.textContent = item;
        checklistElement.appendChild(listItem);
    });

    modal.classList.remove("hidden");
}

function closeEngineModal() {
    modal.classList.add("hidden");
}

function downloadTextFile(filename, content) {
    const blob = new Blob(
        [content],
        { type: "text/plain;charset=utf-8" }
    );

    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");

    link.href = url;
    link.download = filename;

    document.body.appendChild(link);
    link.click();
    link.remove();

    URL.revokeObjectURL(url);
}

function downloadEngineReport() {
    if (!selectedEngine) {
        return;
    }

    const checklist = getChecklist(selectedEngine);

    const report = `
AEROMIND EDGE MAINTENANCE REPORT

Engine ID: ${selectedEngine.engineId}
Current Cycle: ${selectedEngine.currentCycle}
Health Score: ${selectedEngine.healthScore}%
Failure Risk: ${selectedEngine.failureRisk}%
Remaining Useful Life: ${selectedEngine.predictedRul} cycles
Risk Level: ${selectedEngine.riskLevel}
Confidence: ${calculateConfidence(selectedEngine)}%

Detected Condition:
${selectedEngine.detectedIssue}

Maintenance Copilot Insight:
${selectedEngine.explanation}

Recommended Action:
${selectedEngine.recommendation}

Inspection Checklist:
${checklist.map((item) => `- ${item}`).join("\n")}

Dataset:
NASA C-MAPSS FD001

Generated by AeroMind Edge
`.trim();

    downloadTextFile(
        `${selectedEngine.engineId}-maintenance-report.txt`,
        report
    );
}

function downloadFleetReport() {
    const criticalEngines = engineData.filter(
        (engine) => engine.riskLevel === "CRITICAL"
    );

    const moderateEngines = engineData.filter(
        (engine) => engine.riskLevel === "MODERATE"
    );

    const averageHealth = engineData.length
        ? (
            engineData.reduce(
                (sum, engine) => sum + engine.healthScore,
                0
            ) / engineData.length
        ).toFixed(1)
        : "0.0";

    const report = `
AEROMIND EDGE FLEET REPORT

Dataset: NASA C-MAPSS FD001
Total Engines: ${engineData.length}
Average Fleet Health: ${averageHealth}%
Critical Engines: ${criticalEngines.length}
Moderate-Risk Engines: ${moderateEngines.length}

CRITICAL ENGINE QUEUE
${criticalEngines
    .map(
        (engine) =>
            `${engine.engineId} | RUL: ${engine.predictedRul} cycles | Risk: ${engine.failureRisk}%`
    )
    .join("\n")}

Generated by AeroMind Edge
`.trim();

    downloadTextFile(
        "AeroMind-Fleet-Report.txt",
        report
    );
}

function toggleTheme() {
    document.body.classList.toggle("dark-mode");

    const isDark =
        document.body.classList.contains("dark-mode");

    document.getElementById("themeToggle").textContent =
        isDark ? "☀" : "☾";

    localStorage.setItem(
        "aeromind-theme",
        isDark ? "dark" : "light"
    );

    setTimeout(() => {
        renderCharts({
            summary: {
                lowRisk: engineData.filter(
                    (engine) => engine.riskLevel === "LOW"
                ).length,

                moderateRisk: engineData.filter(
                    (engine) => engine.riskLevel === "MODERATE"
                ).length,

                criticalRisk: engineData.filter(
                    (engine) => engine.riskLevel === "CRITICAL"
                ).length
            }
        });
    }, 50);
}

function restoreTheme() {
    const savedTheme =
        localStorage.getItem("aeromind-theme");

    if (savedTheme === "dark") {
        document.body.classList.add("dark-mode");
        document.getElementById("themeToggle").textContent = "☀";
    }
}

function showCriticalEngines() {
    riskFilter.value = "CRITICAL";
    filterEngines();

    document.querySelector(".table-container").scrollIntoView({
        behavior: "smooth"
    });
}

function setupNavigation() {
    document.querySelectorAll(".nav-item").forEach((button) => {
        button.addEventListener("click", () => {
            document.querySelectorAll(".nav-item").forEach((item) => {
                item.classList.remove("active");
            });

            button.classList.add("active");

            const section = button.dataset.section;

            if (section === "analytics") {
                document
                    .getElementById("analyticsSection")
                    .scrollIntoView({ behavior: "smooth" });
            }

            if (section === "critical") {
                showCriticalEngines();
            }

            if (section === "overview") {
                window.scrollTo({
                    top: 0,
                    behavior: "smooth"
                });
            }

            if (section === "copilot") {
                const criticalEngine = engineData.find(
                    (engine) => engine.riskLevel === "CRITICAL"
                );

                if (criticalEngine) {
                    openEngineModal(criticalEngine.engineId);
                }
            }
        });
    });
}

searchInput.addEventListener("input", filterEngines);
riskFilter.addEventListener("change", filterEngines);

closeModalButton.addEventListener("click", closeEngineModal);
modalOverlay.addEventListener("click", closeEngineModal);

document
    .getElementById("downloadEngineReport")
    .addEventListener("click", downloadEngineReport);

document
    .getElementById("downloadFleetReport")
    .addEventListener("click", downloadFleetReport);

document
    .getElementById("themeToggle")
    .addEventListener("click", toggleTheme);

document
    .getElementById("viewCriticalButton")
    .addEventListener("click", showCriticalEngines);

document
    .getElementById("acknowledgeButton")
    .addEventListener("click", () => {
        document.getElementById("acknowledgeButton").textContent =
            "Recommendation Acknowledged";

        document.getElementById("acknowledgeButton").disabled =
            true;
    });

restoreTheme();
setupNavigation();
loadDashboard();