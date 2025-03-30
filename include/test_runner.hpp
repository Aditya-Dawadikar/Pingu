#ifndef TEST_RUNNER_HPP
#define TEST_RUNNER_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_set>

namespace test_runner {
struct TestExecutionResult {
  bool failed;
  int api_time_ms;
  int test_time_ms;
};

TestExecutionResult run_test(const nlohmann::json &testSpec, bool printCompact,
                             int verbosity, std::stringstream &logOut);

} // namespace test_runner

#endif