#ifndef JSON_UTILS_HPP
#define JSON_UTILS_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_set>

namespace json_utils {

bool read_json(const std::string &path, nlohmann::json &out);

bool write_json(const std::string &path, const nlohmann::json &data);

bool diff_json(const nlohmann::json &expected, const nlohmann::json &actual,
               const std::string &path,
               const std::unordered_set<std::string> &ignore,
               const std::unordered_set<std::string> &watch, int indent,
               int level);

bool diff_json_compact(const nlohmann::json &expected,
                       const nlohmann::json &actual, const std::string &path,
                       const std::unordered_set<std::string> &ignore,
                       const std::unordered_set<std::string> &watch);

bool diff_json_collect(const nlohmann::json &expected,
                       const nlohmann::json &actual, std::stringstream &output,
                       const std::string &path,
                       const std::unordered_set<std::string> &ignore,
                       const std::unordered_set<std::string> &watch,
                       int indent, int level);

} // namespace json_utils

#endif