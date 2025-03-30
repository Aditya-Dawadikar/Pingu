#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>

#include "json_utils.hpp"
#include "http_utils.hpp"
#include "test_runner.hpp"

struct TestResult {
    std::string name;
    bool failed;
    std::stringstream log;
};

int main(int argc, char* argv[]) {
    bool printCompact = false;
    bool isTestSuite = false;
    int verbosity = 1;
    bool runInParallel = false;
    std::string testSpecPath;

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
        }
    }

    if (testSpecPath.empty()) {
        std::cerr << "Usage: " << argv[0]
                  << (isTestSuite ? " --test_suit <suite.json>" : " --test <test.json>")
                  << " [--compact] [--parallel] [--verbosity <level>]\n";
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

        // Print collected logs
        for (const auto& result : testLogs) {
            std::cout << result.log.str();
        }

        std::cout << "\nPassed: " << passed << " | Failed: " << failed << "\n";
    } else {
        std::stringstream ss;
        bool failed = test_runner::run_test(testSpec, printCompact, verbosity, ss);
        std::cout << ss.str();
        std::cout << (failed ? "Test Failed\n" : "Test Passed\n");
    }

    return 0;
}
