#ifndef TEST_RUNNER_HPP
#define TEST_RUNNER_HPP

#include <string>
#include <nlohmann/json.hpp>
#include <unordered_set>

namespace test_runner{
    // bool run_test(const nlohmann::json& request_desc, bool printCompact, int verbosity);
    bool run_test(const nlohmann::json& testSpec, bool printCompact, int verbosity, std::stringstream& logOut);

}

#endif