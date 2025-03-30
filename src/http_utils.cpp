#include "http_utils.hpp"
#include <curl/curl.h>
#include <iostream>
#include <sstream>

namespace http_utils {

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  std::string *response = static_cast<std::string *>(userp);
  size_t totalSize = size * nmemb;
  response->append(static_cast<char *>(contents), totalSize);
  return totalSize;
}

bool make_request_from_json(const nlohmann::json &request_desc,
                            nlohmann::json &response_out) {
  CURL *curl = curl_easy_init();
  if (!curl)
    return false;

  std::string responseStr;
  curl_easy_setopt(curl, CURLOPT_URL,
                   request_desc["url"].get<std::string>().c_str());

  // Method
  std::string method = request_desc.value("method", "GET");
  if (method == "POST") {
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
  } else if (method != "GET") {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
  }

  // Headers
  struct curl_slist *headers = NULL;
  if (request_desc.contains("headers")) {
    for (auto &[k, v] : request_desc["headers"].items()) {
      std::string h = k + ": " + v.get<std::string>();
      headers = curl_slist_append(headers, h.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }

  // Body
  std::string bodyStr;
  if (request_desc.contains("body")) {
    bodyStr = request_desc["body"].dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyStr.c_str());
  }

  // Response capture
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  if (headers)
    curl_slist_free_all(headers);

  if (res != CURLE_OK) {
    std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
    return false;
  }

  try {
    response_out = nlohmann::json::parse(responseStr);
  } catch (...) {
    response_out = responseStr; // fallback to raw string
  }

  return true;
}

} // namespace http_utils
