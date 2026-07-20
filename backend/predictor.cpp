#include "predictor.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace {

double clampValue(double value, double minimum, double maximum) {
    return std::max(minimum, std::min(value, maximum));
}

double averageSensor(
    const std::vector<EngineRecord>& history,
    int sensorIndex,
    int startIndex
) {
    double total = 0.0;
    int count = 0;

    for (int index = startIndex;
         index < static_cast<int>(history.size());
         ++index) {

        if (sensorIndex < static_cast<int>(history[index].sensors.size())) {
            total += history[index].sensors[sensorIndex];
            ++count;
        }
    }

    if (count == 0) {
        return 0.0;
    }

    return total / count;
}

double calculateTrend(
    const std::vector<EngineRecord>& history,
    int sensorIndex
) {
    if (history.size() < 10) {
        return 0.0;
    }

    const int sampleSize =
        std::min(10, static_cast<int>(history.size() / 2));

    const double earlyAverage =
        averageSensor(history, sensorIndex, 0);

    const double recentAverage =
        averageSensor(
            history,
            sensorIndex,
            static_cast<int>(history.size()) - sampleSize
        );

    if (std::abs(earlyAverage) < 0.000001) {
        return 0.0;
    }

    return (recentAverage - earlyAverage) /
           std::abs(earlyAverage);
}

std::string buildExplanation(
    int currentCycle,
    int predictedRul,
    double temperatureTrend,
    double pressureTrend,
    double efficiencyTrend
) {
    std::ostringstream text;

    text << "The engine has completed "
         << currentCycle
         << " operating cycles. ";

    text << "Its estimated remaining useful life is "
         << predictedRul
         << " cycles. ";

    text << "Recent sensor trends show ";

    bool addedObservation = false;

    if (temperatureTrend > 0.003) {
        text << "rising thermal load";
        addedObservation = true;
    }

    if (pressureTrend < -0.002) {
        if (addedObservation) {
            text << ", ";
        }

        text << "declining pressure performance";
        addedObservation = true;
    }

    if (efficiencyTrend < -0.002) {
        if (addedObservation) {
            text << ", ";
        }

        text << "reduced engine efficiency";
        addedObservation = true;
    }

    if (!addedObservation) {
        text << "no severe short-term degradation";
    }

    text << ".";

    return text.str();
}

}

EngineSummary analyseEngine(
    int unitNumber,
    const std::vector<EngineRecord>& history,
    int referenceRul
) {
    EngineSummary summary{};

    summary.unitNumber = unitNumber;

    if (history.empty()) {
        summary.currentCycle = 0;
        summary.predictedRul = 0;
        summary.healthScore = 0;
        summary.failureRisk = 100;
        summary.riskLevel = "CRITICAL";
        summary.detectedIssue = "No engine history available";
        summary.recommendation =
            "Inspect the dataset and sensor connection.";
        summary.explanation =
            "The engine could not be analysed because no records were found.";

        return summary;
    }

    const EngineRecord& latestRecord = history.back();

    summary.currentCycle = latestRecord.cycle;

    const double temperatureTrend =
        calculateTrend(history, 3);

    const double pressureTrend =
        calculateTrend(history, 6);

    const double efficiencyTrend =
        calculateTrend(history, 20);

    double trendPenalty = 0.0;

    if (temperatureTrend > 0.0) {
        trendPenalty +=
            clampValue(temperatureTrend * 800.0, 0.0, 20.0);
    }

    if (pressureTrend < 0.0) {
        trendPenalty +=
            clampValue(std::abs(pressureTrend) * 900.0, 0.0, 20.0);
    }

    if (efficiencyTrend < 0.0) {
        trendPenalty +=
            clampValue(std::abs(efficiencyTrend) * 900.0, 0.0, 20.0);
    }

    int estimatedRul = referenceRul;

    if (estimatedRul < 0) {
        estimatedRul =
            std::max(
                5,
                static_cast<int>(
                    170.0 -
                    (summary.currentCycle * 0.55) -
                    trendPenalty
                )
            );
    }

    summary.predictedRul = estimatedRul;

    const double rulHealth =
        clampValue(
            static_cast<double>(estimatedRul) / 130.0,
            0.0,
            1.0
        );

    const double cyclePenalty =
        clampValue(
            static_cast<double>(summary.currentCycle) / 350.0,
            0.0,
            1.0
        );

    double health =
        (rulHealth * 75.0) +
        ((1.0 - cyclePenalty) * 25.0) -
        trendPenalty;

    summary.healthScore =
        static_cast<int>(
            std::round(
                clampValue(health, 0.0, 100.0)
            )
        );

    summary.failureRisk =
        100 - summary.healthScore;

    if (summary.predictedRul <= 30 ||
        summary.failureRisk >= 70) {

        summary.riskLevel = "CRITICAL";
        summary.detectedIssue =
            "Advanced engine degradation detected";

        summary.recommendation =
            "Remove the engine from normal service and perform "
            "priority inspection of turbine, compressor and "
            "lubrication systems.";

    } else if (summary.predictedRul <= 70 ||
               summary.failureRisk >= 40) {

        summary.riskLevel = "MODERATE";
        summary.detectedIssue =
            "Developing engine degradation pattern";

        summary.recommendation =
            "Schedule a detailed inspection during the next "
            "maintenance window and increase sensor monitoring.";

    } else {

        summary.riskLevel = "LOW";
        summary.detectedIssue =
            "No immediate critical fault detected";

        summary.recommendation =
            "Continue normal operation and follow the planned "
            "preventive maintenance schedule.";
    }

    summary.explanation =
        buildExplanation(
            summary.currentCycle,
            summary.predictedRul,
            temperatureTrend,
            pressureTrend,
            efficiencyTrend
        );

    return summary;
}