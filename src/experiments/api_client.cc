#include "api_client.h"

#include <curl/curl.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

APIClient::APIClient(const std::string &apiToken,
                     const std::string &experimentKey) {
  _apiToken = apiToken;
  _experimentKey = experimentKey;
}

// Callback function to handle data received from the server
size_t APIClient::WriteCallback(void *contents, size_t size, size_t nmemb,
                                void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

// Function to make HTTP requests with custom headers and body
std::string APIClient::MakeRequest(const std::string &url,
                                   const std::string &method,
                                   const std::string &body,
                                   const std::vector<std::string> &headers) {
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    struct curl_slist *headerList = nullptr;

    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // Set callback function to handle response data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    // Set HTTP method and body
    if (method == "POST") {
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    } else if (method == "PUT") {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    } else if (method == "DELETE") {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }  // Default is GET

    // Set custom headers
    for (const auto &header : headers) {
      headerList = curl_slist_append(headerList, header.c_str());
    }
    if (headerList) {
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }

    // Perform the request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }

    // Clean up headers list
    if (headerList) {
      curl_slist_free_all(headerList);
    }
    // Clean up CURL
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();

  return readBuffer;
}

std::string APIClient::VectorToJSON(
    const std::vector<std::vector<std::string>> &vec) {
  std::ostringstream jsonStream;
  jsonStream << "{";
  for (size_t i = 0; i < vec.size(); ++i) {
    jsonStream << "\"" << vec[i][0] << "\": \"" << vec[i][1] << "\"";
    if (i < vec.size() - 1) {
      jsonStream << ", ";
    }
  }
  jsonStream << "}";
  return jsonStream.str();
}

void APIClient::LogParameter(const std::string &parameterName,
                             const std::string &parameterValue,
                             const std::string &step,
                             const std::string &timestamp) {
  std::string url = _baseUrl + "/api/rest/v2/write/experiment/parameter";

  std::vector<std::vector<std::string>> bodyVec = {
      {"experimentKey", _experimentKey},
      {"parameterName", parameterName},
      {"parameterValue", parameterValue}};
  if (!step.empty()) {
    bodyVec.push_back({"step", step});
  }
  if (!timestamp.empty()) {
    bodyVec.push_back({"timestamp", timestamp});
  }
  std::string body = VectorToJSON(bodyVec);

  std::vector<std::string> headers = {"Authorization: " + _apiToken,
                                      "Content-Type: application/json"};

  std::string res = MakeRequest(url, "POST", body, headers);
}

void APIClient::LogMetric(const std::string &metricName,
                          const std::string &metricValue,
                          const std::string &context, const std::string &step,
                          const std::string &epoch,
                          const std::string &timestamp) {
  std::string url = _baseUrl + "/api/rest/v2/write/experiment/metric";

  std::vector<std::vector<std::string>> bodyVec = {
      {"experimentKey", _experimentKey},
      {"metricName", metricName},
      {"metricValue", metricValue}};
  if (!context.empty()) {
    bodyVec.push_back({"context", context});
  }
  if (!step.empty()) {
    bodyVec.push_back({"step", step});
  }
  if (!epoch.empty()) {
    bodyVec.push_back({"epoch", epoch});
  }
  if (!timestamp.empty()) {
    bodyVec.push_back({"timestamp", timestamp});
  }
  std::string body = VectorToJSON(bodyVec);

  std::vector<std::string> headers = {"Authorization: " + _apiToken,
                                      "Content-Type: application/json"};

  std::string res = MakeRequest(url, "POST", body, headers);
}