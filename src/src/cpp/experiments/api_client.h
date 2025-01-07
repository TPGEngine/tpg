#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <string>
#include <vector>

class APIClient {
 public:
  APIClient(const std::string &apiToken, const std::string &experimentKey);

  static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                              void *userp);

  std::string MakeRequest(const std::string &url, const std::string &method,
                          const std::string &body,
                          const std::vector<std::string> &headers);

  std::string VectorToJSON(const std::vector<std::vector<std::string>> &vec);

  void LogParameter(const std::string &parameterName,
                    const std::string &parameterValue,
                    const std::string &step = "",
                    const std::string &timestamp = "");

  void LogMetric(const std::string &metricName, const std::string &metricValue,
                 const std::string &context = "", const std::string &step = "",
                 const std::string &epoch = "",
                 const std::string &timestamp = "");

 private:
  std::string _baseUrl = "https://www.comet.com";
  std::string _apiToken;
  std::string _experimentKey;
};

#endif  // API_CLIENT_H