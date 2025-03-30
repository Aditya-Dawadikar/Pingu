#ifndef TEST_RUNNER_HPP
#define TEST_RUNNER_HPP

#include <string>
#include <nlohmann/json.hpp>
#include <unordered_set>

namespace test_runner{
    struct TestExecutionResult {
        bool failed;
        int api_time_ms;
        int test_time_ms;
    };
    
    TestExecutionResult run_test(
        const nlohmann::json& testSpec,
        bool printCompact,
        int verbosity,
        std::stringstream& logOut
    );

}

#endif