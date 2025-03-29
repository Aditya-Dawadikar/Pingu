#include <iostream>
#include <string>
#include "json_utils.hpp"
#include "http_utils.hpp"
#include "test_runner.hpp"

int main(int argc, char* argv[]) {

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // LOGIC TO RUN API TEST
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    bool printCompact = false;
    bool isTestSuite = false;
    int verbosity = 1;

    std::string testSpecPath;

    for(int i=1;i<argc;i++){
        if(std::string(argv[i])=="--test"){
            testSpecPath = argv[i+1];
        }
        else if(std::string(argv[i])=="--test_suit"){
            isTestSuite = true;
            testSpecPath = argv[i+1];
        }
        else if(std::string(argv[i])=="--compact"){
            printCompact = true;
        }
        else if(std::string(argv[i]) == "--verbosity" && i+1 < argc){
            verbosity = std::stoi(argv[i+1]);
        }
    }

    if (testSpecPath.empty()){
        std::cerr << "Usage: " << argv[0]
                  << (isTestSuite ? " --test_suit <suite.json>" : " --test <test.json>")
                  << " [--compact]\n";
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

        int passed = 0, failed = 0;
        for (const auto& testCase : testSpec["test_cases"]) {
            if (test_runner::run_test(testCase, printCompact, verbosity)) {
                failed++;
            } else {
                passed++;
            }
        }

        std::cout << "\nPassed: " << passed << " | Failed: " << failed << "\n";
    } else {
        test_runner::run_test(testSpec, printCompact, verbosity);
    }

    return 0;
}
