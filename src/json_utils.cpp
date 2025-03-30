#include "json_utils.hpp"
#include <fstream>
#include <iostream>
#include <set>
#include <unordered_set>
#include <sstream>
#include "log_utils.hpp"

namespace json_utils {

    bool read_json(const std::string& path, nlohmann::json& out) {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        try { file >> out; }
        catch (...) { return false; }
        return true;
    }

    bool write_json(const std::string& path, const nlohmann::json& data) {
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << data.dump(4);
        return true;
    }

    // Internal recursive diff that writes to a stringstream instead of directly printing
    bool diff_json_collect(
        const nlohmann::json& expected,
        const nlohmann::json& actual,
        std::stringstream& output,
        const std::string& path,
        const std::unordered_set<std::string>& ignore,
        int indent,
        int level
    ) {
        std::set<std::string> keys;
        if (expected.is_object()) {
            for (auto& [k, _] : expected.items()) keys.insert(k);
        }
        if (actual.is_object()) {
            for (auto& [k, _] : actual.items()) keys.insert(k);
        }

        std::string pad(level * indent, ' ');
        bool hasDifference = false;

        for (const auto& key : keys) {
            std::string fullpath = path.empty() ? key : path + "." + key;
            if (ignore.find(fullpath) != ignore.end()) continue;

            bool inExpected = expected.contains(key);
            bool inActual = actual.contains(key);

            const auto& eVal = inExpected ? expected.at(key) : nlohmann::json();
            const auto& aVal = inActual ? actual.at(key) : nlohmann::json();

            if (inExpected && !inActual) {
                hasDifference = true;
                output << pad << std::string(indent, ' ')
                       << COLOR_RED << "- \"" << key << "\": " << eVal.dump() << COLOR_RESET << ",\n";
                continue;
            }

            if (!inExpected && inActual) {
                hasDifference = true;
                output << pad << std::string(indent, ' ')
                       << COLOR_GREEN << "+ \"" << key << "\": " << aVal.dump() << COLOR_RESET << ",\n";
                continue;
            }

            if (eVal.is_object() && aVal.is_object()) {
                std::stringstream nested;
                bool nestedDiff = diff_json_collect(eVal, aVal, nested, fullpath, ignore, indent, level + 1);
                if (nestedDiff) {
                    hasDifference = true;
                    output << pad << std::string(indent, ' ')
                           << "\"" << key << "\": {\n"
                           << nested.str()
                           << pad << std::string(indent, ' ') << "},\n";
                } else {
                    output << pad << std::string(indent, ' ')
                           << "\"" << key << "\": " << aVal.dump() << ",\n";
                }
            } else if (eVal != aVal) {
                hasDifference = true;
                output << pad << std::string(indent, ' ')
                       << COLOR_RED << "- \"" << key << "\": " << eVal.dump() << "," << COLOR_RESET << "\n";
                output << pad << std::string(indent, ' ')
                       << COLOR_GREEN << "+ \"" << key << "\": " << aVal.dump() << "," << COLOR_RESET << "\n";
            } else {
                output << pad << std::string(indent, ' ')
                       << "\"" << key << "\": " << aVal.dump() << ",\n";
            }
        }

        return hasDifference;
    }

    // Public wrapper that prints the collected diff
    bool diff_json(const nlohmann::json& expected,
                   const nlohmann::json& actual,
                   const std::string& path,
                   const std::unordered_set<std::string>& ignore,
                   int indent,
                   int level) {
        std::stringstream finalOutput;
        bool hasDiff = diff_json_collect(expected, actual, finalOutput, path, ignore, indent, level);
        if (hasDiff) {
            std::string pad(level * indent, ' ');
            std::cout << pad << "{\n" << finalOutput.str() << pad << "}";
            if (level > 0) std::cout << ",\n"; else std::cout << "\n";
        }
        return hasDiff;
    }

    // Compact diff that logs line-by-line for simple structure
    bool diff_json_compact(const nlohmann::json& expected,
                           const nlohmann::json& actual,
                           const std::string& path,
                           const std::unordered_set<std::string>& ignore) {
        std::set<std::string> keys;
        if (expected.is_object()) {
            for (auto& [k, _] : expected.items()) keys.insert(k);
        }
        if (actual.is_object()) {
            for (auto& [k, _] : actual.items()) keys.insert(k);
        }

        bool hasDifference = false;

        for (const auto& key : keys) {
            std::string fullpath = path.empty() ? key : path + "." + key;
            if (ignore.find(fullpath) != ignore.end()) continue;

            bool inExpected = expected.contains(key);
            bool inActual = actual.contains(key);

            const auto& eVal = inExpected ? expected.at(key) : nlohmann::json();
            const auto& aVal = inActual ? actual.at(key) : nlohmann::json();

            if (inExpected && inActual && eVal.is_object() && aVal.is_object()) {
                if (diff_json_compact(eVal, aVal, fullpath, ignore)) {
                    hasDifference = true;
                }
            } else if (inExpected && !inActual) {
                hasDifference = true;
                std::cout << COLOR_RED << "-\"" << fullpath << "\":" << eVal.dump() << COLOR_RESET << "\n";
            } else if (!inExpected && inActual) {
                hasDifference = true;
                std::cout << COLOR_GREEN << "+\"" << fullpath << "\":" << aVal.dump() << COLOR_RESET << "\n";
            } else if (eVal != aVal) {
                hasDifference = true;
                std::cout << COLOR_RED << "-\"" << fullpath << "\":" << eVal.dump() << COLOR_RESET << "\n";
                std::cout << COLOR_GREEN << "+\"" << fullpath << "\":" << aVal.dump() << COLOR_RESET << "\n";
            }
        }

        return hasDifference;
    }

} // namespace json_utils
