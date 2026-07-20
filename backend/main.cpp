#include "predictor.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> values;
    std::stringstream stream(line);
    std::string value;

    while (std::getline(stream, value, ',')) {
        values.push_back(value);
    }

    return values;
}

std::string escapeJson(const std::string& text) {
    std::string result;

    for (char character : text) {
        switch (character) {
            case '"':
                result += "\\\"";
                break;

            case '\\':
                result += "\\\\";
                break;

            case '\n':
                result += "\\n";
                break;

            case '\r':
                result += "\\r";
                break;

            case '\t':
                result += "\\t";
                break;

            default:
                result += character;
                break;
        }
    }

    return result;
}

EngineRecord parseEngineRecord(
    const std::vector<std::string>& values
) {
    if (values.size() < 26) {
        throw std::runtime_error(
            "Expected 26 CSV columns but received " +
            std::to_string(values.size())
        );
    }

    EngineRecord record{};

    record.unitNumber = std::stoi(values[0]);
    record.cycle = std::stoi(values[1]);
    record.setting1 = std::stod(values[2]);
    record.setting2 = std::stod(values[3]);
    record.setting3 = std::stod(values[4]);

    for (int column = 5; column < 26; ++column) {
        record.sensors.push_back(
            std::stod(values[column])
        );
    }

    return record;
}

std::map<int, std::vector<EngineRecord>>
loadEngineDataset(const std::string& filePath) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Could not open dataset: " + filePath
        );
    }

    std::map<int, std::vector<EngineRecord>> engines;

    std::string line;

    if (!std::getline(file, line)) {
        throw std::runtime_error(
            "Dataset is empty: " + filePath
        );
    }

    int rowNumber = 1;

    while (std::getline(file, line)) {
        ++rowNumber;

        if (line.empty()) {
            continue;
        }

        try {
            const std::vector<std::string> values =
                splitCsvLine(line);

            EngineRecord record =
                parseEngineRecord(values);

            engines[record.unitNumber].push_back(record);

        } catch (const std::exception& error) {
            std::cerr
                << "Skipping row "
                << rowNumber
                << ": "
                << error.what()
                << '\n';
        }
    }

    if (engines.empty()) {
        throw std::runtime_error(
            "No valid engine records were found."
        );
    }

    return engines;
}

std::vector<int> loadRulValues(
    const std::string& filePath
) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Could not open RUL file: " + filePath
        );
    }

    std::vector<int> values;
    std::string line;

    if (!std::getline(file, line)) {
        return values;
    }

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream stream(line);
        std::string value;

        if (std::getline(stream, value, ',')) {
            try {
                values.push_back(std::stoi(value));
            } catch (...) {
                std::cerr
                    << "Skipping invalid RUL value: "
                    << value
                    << '\n';
            }
        }
    }

    return values;
}

void printSummary(
    const std::vector<EngineSummary>& summaries
) {
    std::cout << "\n";
    std::cout
        << "==========================================================\n";

    std::cout
        << "               AEROMIND EDGE FLEET REPORT\n";

    std::cout
        << "==========================================================\n";

    for (const EngineSummary& engine : summaries) {
        std::cout
            << "\nEngine: FD001-"
            << std::setw(3)
            << std::setfill('0')
            << engine.unitNumber
            << std::setfill(' ')
            << '\n';

        std::cout
            << "Current cycle: "
            << engine.currentCycle
            << '\n';

        std::cout
            << "Estimated RUL: "
            << engine.predictedRul
            << " cycles\n";

        std::cout
            << "Health score: "
            << engine.healthScore
            << "%\n";

        std::cout
            << "Failure risk: "
            << engine.failureRisk
            << "%\n";

        std::cout
            << "Risk level: "
            << engine.riskLevel
            << '\n';

        std::cout
            << "Issue: "
            << engine.detectedIssue
            << '\n';

        std::cout
            << "Recommendation: "
            << engine.recommendation
            << '\n';

        std::cout
            << "----------------------------------------------------------\n";
    }
}

void writeDashboardJson(
    const std::string& outputPath,
    const std::vector<EngineSummary>& summaries
) {
    std::ofstream file(outputPath);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Could not create dashboard file: " + outputPath
        );
    }

    int lowRisk = 0;
    int moderateRisk = 0;
    int criticalRisk = 0;
    double totalHealth = 0.0;

    for (const EngineSummary& engine : summaries) {
        totalHealth += engine.healthScore;

        if (engine.riskLevel == "LOW") {
            ++lowRisk;
        } else if (engine.riskLevel == "MODERATE") {
            ++moderateRisk;
        } else {
            ++criticalRisk;
        }
    }

    const double averageHealth =
        summaries.empty()
            ? 0.0
            : totalHealth /
              static_cast<double>(summaries.size());

    file << "{\n";
    file << "  \"project\": \"AeroMind Edge\",\n";
    file << "  \"dataset\": \"NASA C-MAPSS FD001\",\n";
    file << "  \"status\": \"Operational\",\n";

    file << "  \"summary\": {\n";
    file << "    \"totalEngines\": "
         << summaries.size()
         << ",\n";

    file << "    \"averageHealth\": "
         << std::fixed
         << std::setprecision(1)
         << averageHealth
         << ",\n";

    file << "    \"lowRisk\": "
         << lowRisk
         << ",\n";

    file << "    \"moderateRisk\": "
         << moderateRisk
         << ",\n";

    file << "    \"criticalRisk\": "
         << criticalRisk
         << "\n";

    file << "  },\n";
    file << "  \"engines\": [\n";

    for (std::size_t index = 0;
         index < summaries.size();
         ++index) {

        const EngineSummary& engine =
            summaries[index];

        file << "    {\n";

        file << "      \"engineId\": \"FD001-"
             << std::setw(3)
             << std::setfill('0')
             << engine.unitNumber
             << std::setfill(' ')
             << "\",\n";

        file << "      \"currentCycle\": "
             << engine.currentCycle
             << ",\n";

        file << "      \"predictedRul\": "
             << engine.predictedRul
             << ",\n";

        file << "      \"healthScore\": "
             << engine.healthScore
             << ",\n";

        file << "      \"failureRisk\": "
             << engine.failureRisk
             << ",\n";

        file << "      \"riskLevel\": \""
             << escapeJson(engine.riskLevel)
             << "\",\n";

        file << "      \"detectedIssue\": \""
             << escapeJson(engine.detectedIssue)
             << "\",\n";

        file << "      \"recommendation\": \""
             << escapeJson(engine.recommendation)
             << "\",\n";

        file << "      \"explanation\": \""
             << escapeJson(engine.explanation)
             << "\"\n";

        file << "    }";

        if (index + 1 < summaries.size()) {
            file << ",";
        }

        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";
}

}

int main() {
    try {
        const std::string testDataset =
            "data/test/test_FD001.csv";

        const std::string rulDataset =
            "data/rul/RUL_FD001.csv";

        const std::string dashboardOutput =
            "frontend/data.json";

        const auto engines =
            loadEngineDataset(testDataset);

        const std::vector<int> rulValues =
            loadRulValues(rulDataset);

        std::vector<EngineSummary> summaries;
        summaries.reserve(engines.size());

        for (const auto& entry : engines) {
            const int unitNumber = entry.first;
            const std::vector<EngineRecord>& history =
                entry.second;

            int referenceRul = -1;

            const int rulIndex = unitNumber - 1;

            if (rulIndex >= 0 &&
                rulIndex <
                    static_cast<int>(rulValues.size())) {

                referenceRul = rulValues[rulIndex];
            }

            summaries.push_back(
                analyseEngine(
                    unitNumber,
                    history,
                    referenceRul
                )
            );
        }

        printSummary(summaries);

        writeDashboardJson(
            dashboardOutput,
            summaries
        );

        std::cout << "\nAnalysis completed successfully.\n";
        std::cout
            << "Dashboard data created at: "
            << dashboardOutput
            << "\n";

        return 0;

    } catch (const std::exception& error) {
        std::cerr
            << "\nAeroMind error: "
            << error.what()
            << '\n';

        return 1;
    }
}