#pragma once

#include <curl/curl.h>
#include <string>

struct HTTPResponse {
  unsigned char* data;
  size_t size;
  long status;
};

class HTTPRequest {
  public:
    HTTPRequest(CURL* request, CURLM* handler);
    ~HTTPRequest();
    bool isReady;
    CURL* curl;
    struct curl_slist *headers;
    CURLM* handler;
    HTTPResponse response;
};

class HTTP {
  public:
    HTTP();
    ~HTTP();
    int count;
    HTTPRequest* request(std::string url, const std::string& body = "");
    void update();
    static bool isValidURL(const std::string& url);
  private:
    static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
    const struct curl_blob ca;
    CURLM* handler;
};
