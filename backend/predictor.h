#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <string>
#include <vector>

struct EngineRecord {
    int unitNumber;
    int cycle;
    double setting1;
    double setting2;
    double setting3;
    std::vector<double> sensors;
};

struct EngineSummary {
    int unitNumber;
    int currentCycle;
    int predictedRul;
    int healthScore;
    int failureRisk;
    std::string riskLevel;
    std::string detectedIssue;
    std::string recommendation;
    std::string explanation;
};

EngineSummary analyseEngine(
    int unitNumber,
    const std::vector<EngineRecord>& history,
    int referenceRul
);

#endif