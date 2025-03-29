#ifndef TEST_RUNNER_HPP
#define TEST_RUNNER_HPP

#include <string>
#include <nlohmann/json.hpp>
#include <unordered_set>

namespace test_runner{
    bool run_test(const nlohmann::json& request_desc, bool printCompact, int verbosity);
}

#endif