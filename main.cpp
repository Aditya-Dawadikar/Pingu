#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <fstream>

#include "json_utils.hpp"
#include "http_utils.hpp"
#include "test_runner.hpp"

struct TestResult {
    std::string name;
    bool failed;
    std::stringstream log;
};

void print_help() {
    std::cout << R"(
    .--.      
   |o_o |     
   |:_/ |     ____ ___ _   _  ____ _   _ 
  //   \ \   |  _ \_ _| \ | |/ ___| | | |
 (|     | )  | |_) | ||  \| | |  _| | | |
/'\_   _/`\  |  __/| || |\  | |_| | |_| |
\___)=(___/  |_|  |___|_| \_|\____|\___/

            REST API Testing CLI
--------------------------------

Usage:
  pingu --test <test.json> [options]
  pingu --test_suit <suite.json> [options]

Options:
  --test <json_file>           Run a single test case from test spec file.
  --test_suit <json_file>      Run a full test suite with multiple cases.
  --compact               Print diff output in compact style.
  --parallel              Run all tests in parallel (use with --test_suit).
  --verbosity <level>     Verbosity level (0 = minimal, 1 = default, 2 = detailed).
  --export-log <json_file>     Export test results and logs to JSON file.
  --help                  Show this help message.

Examples:
  pingu --test test.json --compact
  pingu --test_suit suite.json --parallel --export-log results.json


)";
}


int main(int argc, char* argv[]) {
    bool printCompact = false;
    bool isTestSuite = false;
    int verbosity = 1;
    bool runInParallel = false;
    std::string testSpecPath;
    std::string exportPath;
    std::string pingUrl;
    int pingTimeoutMs = 5000;
    int pingRetries = 1;

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--test") {
            testSpecPath = argv[i + 1];
        } else if (std::string(argv[i]) == "--test_suit") {
            isTestSuite = true;
            testSpecPath = argv[i + 1];
        } else if (std::string(argv[i]) == "--parallel") {
            runInParallel = true;
        } else if (std::string(argv[i]) == "--compact") {
            printCompact = true;
        } else if (std::string(argv[i]) == "--verbosity" && i + 1 < argc) {
            verbosity = std::stoi(argv[i + 1]);
        } else if (std::string(argv[i]) == "--export-log" && i + 1 < argc) {
            exportPath = argv[i + 1];
        } else if(std::string(argv[i]) == "--help"){
            print_help();
            return 0;
        }else if (std::string(argv[i]) == "--ping" && i + 1 < argc) {
            pingUrl = argv[i + 1];
            break; // We can exit early since this is a standalone command
        } else if (std::string(argv[i]) == "--ping-timeout" && i + 1 < argc) {
            pingTimeoutMs = std::stoi(argv[++i]);
        } else if (std::string(argv[i]) == "--ping-retries" && i + 1 < argc) {
            pingRetries = std::stoi(argv[++i]);
        }
    }

    if (!pingUrl.empty()) {
        bool success = false;
    
        for (int attempt = 1; attempt <= pingRetries; ++attempt) {
            std::cout << "Pinging " << pingUrl << " (attempt " << attempt << " of " << pingRetries << ")...\n";
    
            nlohmann::json response;
            nlohmann::json dummyRequest = {
                {"method", "GET"},
                {"url", pingUrl},
                {"timeout", pingTimeoutMs}
            };
    
            auto start = std::chrono::high_resolution_clock::now();
            success = http_utils::make_request_from_json(dummyRequest, response);
            auto end = std::chrono::high_resolution_clock::now();
    
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
            if (success) {
                std::cout << "Response received in " << ms << "ms\n";
                return 0;
            } else {
                std::cerr << "No response. Retrying...\n";
            }
        }
    
        std::cerr << "Failed to ping " << pingUrl << " after " << pingRetries << " attempts.\n";
        return 1;
    }
        

    if (testSpecPath.empty()) {
        std::cerr << "Usage: " << argv[0]
                  << (isTestSuite ? " --test_suit <suite.json>" : " --test <test.json>")
                  << " [--compact] [--parallel] [--verbosity <level>] [--export-log <file>]\n";
        return 1;
    }

    nlohmann::json testSpec;
    if (!json_utils::read_json(testSpecPath, testSpec)) {
        std::cerr << "Failed to read JSON from " << testSpecPath << "\n";
        return 1;
    }

    if (isTestSuite) {
        if (!testSpec.contains("test_cases") || !testSpec["test_cases"].is_array()) {
            std::cerr << "Invalid test suite: missing or invalid 'test_cases' array\n";
            return 1;
        }

        if (verbosity > 0) {
            std::cout << "Running Test Suite: " << testSpec["test_suit_name"] << "\n";
            if (verbosity > 1 && !testSpec["test_suit_description"].empty()) {
                std::cout << "Description: " << testSpec["test_suit_description"] << "\n";
            }
        }

        std::vector<TestResult> testLogs;
        std::mutex log_mutex;
        int passed = 0, failed = 0;

        if (runInParallel) {
            std::vector<std::thread> threads;

            for (const auto& testCase : testSpec["test_cases"]) {
                threads.emplace_back([&, testCase]() {
                    std::stringstream ss;
                    bool test_failed = test_runner::run_test(testCase, printCompact, verbosity, ss);

                    std::lock_guard<std::mutex> lock(log_mutex);
                    testLogs.push_back({ testCase["test_name"], test_failed, std::move(ss) });
                    if (test_failed) failed++;
                    else passed++;
                });
            }

            for (auto& t : threads) {
                if (t.joinable()) t.join();
            }
        } else {
            for (const auto& testCase : testSpec["test_cases"]) {
                std::stringstream ss;
                bool test_failed = test_runner::run_test(testCase, printCompact, verbosity, ss);
                testLogs.push_back({ testCase["test_name"], test_failed, std::move(ss) });
                if (test_failed) failed++;
                else passed++;
            }
        }

        for (const auto& result : testLogs) {
            std::cout << result.log.str();
        }

        std::cout << "\nPassed: " << passed << " | Failed: " << failed << "\n";

        if (!exportPath.empty()) {
            nlohmann::json exportJson;
            exportJson["test_suite_name"] = testSpec["test_suit_name"];
            exportJson["results"] = nlohmann::json::array();

            for (const auto& result : testLogs) {
                exportJson["results"].push_back({
                    { "test_name", result.name },
                    { "status", result.failed ? "failed" : "passed" },
                    { "log", result.log.str() }
                });
            }

            std::ofstream outFile(exportPath);
            if (outFile.is_open()) {
                outFile << exportJson.dump(2);
                std::cout << "\nExported logs to " << exportPath << "\n";
            } else {
                std::cerr << "Failed to write logs to " << exportPath << "\n";
            }
        }

    } else {
        std::stringstream ss;
        bool test_failed = test_runner::run_test(testSpec, printCompact, verbosity, ss);
        std::cout << ss.str();
        std::cout << (test_failed ? "Test Failed\n" : "Test Passed\n");

        if (!exportPath.empty()) {
            nlohmann::json exportJson;
            exportJson["results"] = nlohmann::json::array();
            exportJson["results"].push_back({
                { "test_name", testSpec["test_name"] },
                { "status", test_failed ? "failed" : "passed" },
                { "log", ss.str() }
            });

            std::ofstream outFile(exportPath);
            if (outFile.is_open()) {
                outFile << exportJson.dump(2);
                std::cout << "\nExported logs to " << exportPath << "\n";
            } else {
                std::cerr << "Failed to write logs to " << exportPath << "\n";
            }
        }
    }

    return 0;
}
