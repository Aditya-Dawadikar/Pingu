#include "http_utils.hpp"
#include "json_utils.hpp"
#include "log_utils.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>

namespace test_runner {

struct TestExecutionResult {
  bool failed;
  int api_time_ms;
  int test_time_ms;
};

TestExecutionResult run_test(const nlohmann::json &testSpec,
                             const bool printCompact, int verbosity,
                             std::stringstream &logOut) {
  std::string requestJSON = testSpec["request_description"];
  std::string responseJSON = testSpec["expected_response"];

  nlohmann::json request_desc, response, expected_response;
  json_utils::read_json(requestJSON, request_desc);
  json_utils::read_json(responseJSON, expected_response);

  std::unordered_set<std::string> ignoreKeys;
  if (testSpec.contains("ignore") && testSpec["ignore"].is_array()) {
    for (const auto &item : testSpec["ignore"]) {
      ignoreKeys.insert(item.get<std::string>());
    }
  }

  std::unordered_set<std::string> watchKeys;
  if (testSpec.contains("watch") && testSpec["watch"].is_array()) {
    for (const auto &item : testSpec["watch"]) {
      watchKeys.insert(item.get<std::string>());
    }
  }

  bool test_failed = false;

  auto test_start = std::chrono::high_resolution_clock::now();

  if (verbosity > 0) {
    logOut << "\n────────────────────────────────────────────────────\n";
    logOut << "[Test] \"" << testSpec["test_name"] << "\"\n";
    if (verbosity > 1 && !testSpec["test_description"].empty()) {
      logOut << "🔹 " << testSpec["test_description"] << "\n";
    }
  }

  auto api_start = std::chrono::high_resolution_clock::now();
  bool api_success = http_utils::make_request_from_json(request_desc, response);
  auto api_end = std::chrono::high_resolution_clock::now();
  int api_time_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(api_end - api_start)
          .count();

  if (api_success) {
    // Redirect std::cout to logOut
    std::streambuf *original_buf = std::cout.rdbuf();
    std::cout.rdbuf(logOut.rdbuf());

    if (printCompact) {
      test_failed = json_utils::diff_json_compact(expected_response, response,
                                                  "", ignoreKeys, watchKeys);
    } else {
      test_failed = json_utils::diff_json(expected_response, response, "",
                                          ignoreKeys, watchKeys, 2, 0);
    }

    // Restore std::cout
    std::cout.rdbuf(original_buf);

    if (test_failed) {
      logOut << COLOR_RED << "Test \"" << testSpec["test_name"] << "\" Failed"
             << COLOR_RESET << "\n";
    } else {
      logOut << COLOR_GREEN << "Test \"" << testSpec["test_name"]
             << "\" Successful" << COLOR_RESET << "\n";
    }
  } else {
    test_failed = true;
    logOut << COLOR_RED << "API Request Failed" << COLOR_RESET << "\n";
  }

  auto test_end = std::chrono::high_resolution_clock::now();
  int test_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                         test_end - test_start)
                         .count();

  logOut << COLOR_BLUE << "\nAPI Time: " << api_time_ms << " ms\n"
         << COLOR_RESET;
  logOut << COLOR_BLUE << "Total Test Time: " << test_time_ms << " ms\n"
         << COLOR_RESET;

  return {test_failed, api_time_ms, test_time_ms};
}

} // namespace test_runner
