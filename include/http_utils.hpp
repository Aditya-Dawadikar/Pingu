#ifndef HTTP_UTILS_HPP
#define HTTP_UTILS_HPP

#include <nlohmann/json.hpp>
#include <string>

namespace http_utils {
bool make_request_from_json(const nlohmann::json &request_desc,
                            nlohmann::json &response_out);
}

#endif