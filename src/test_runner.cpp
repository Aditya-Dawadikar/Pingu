#include "json_utils.hpp"
#include "http_utils.hpp"
#include "log_utils.hpp"

#include <fstream>
#include <iostream>
#include <set>
#include <unordered_set>
#include <string>
#include <sstream>

namespace test_runner {

bool run_test(const nlohmann::json& testSpec, const bool printCompact, int verbosity, std::stringstream& logOut) {
    std::string requestJSON = testSpec["request_description"];
    std::string responseJSON = testSpec["expected_response"];

    nlohmann::json request_desc, response, expected_response;
    json_utils::read_json(requestJSON, request_desc);
    json_utils::read_json(responseJSON, expected_response);

    std::unordered_set<std::string> ignoreKeys;
    if (testSpec.contains("ignore") && testSpec["ignore"].is_array()) {
        for (const auto& item : testSpec["ignore"]) {
            ignoreKeys.insert(item.get<std::string>());
        }
    }

    bool test_failed = false;

    if (verbosity > 0) {
        logOut << "\n────────────────────────────────────────────────────\n";
        logOut << "[Test] \"" << testSpec["test_name"] << "\"\n";
        if (verbosity > 1 && !testSpec["test_description"].empty()) {
            logOut << "🔹 " << testSpec["test_description"] << "\n";
        }
    }

    if (http_utils::make_request_from_json(request_desc, response)) {
        // Redirect std::cout to logOut
        std::streambuf* original_buf = std::cout.rdbuf();
        std::cout.rdbuf(logOut.rdbuf());

        if (printCompact) {
            test_failed = json_utils::diff_json_compact(expected_response, response, "", ignoreKeys);
        } else {
            test_failed = json_utils::diff_json(expected_response, response, "", ignoreKeys, 2, 0);
        }

        // Restore std::cout
        std::cout.rdbuf(original_buf);

        if (test_failed) {
            logOut << COLOR_RED << "Test \"" << testSpec["test_name"] << "\" Failed" << COLOR_RESET << "\n";
        } else {
            logOut << COLOR_GREEN << "Test \"" << testSpec["test_name"] << "\" Successful" << COLOR_RESET << "\n";
        }
    }

    return test_failed;
}

}
