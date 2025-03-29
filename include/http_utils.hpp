#ifndef HTTP_UTILS_HPP
#define HTTP_UTILS_HPP

#include <string>
#include <nlohmann/json.hpp>

namespace http_utils{
    bool make_request_from_json(const nlohmann::json& request_desc, nlohmann::json& response_out);
}

#endif