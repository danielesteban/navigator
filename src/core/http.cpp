#include "http.hpp"
#include "lib/http_ca.hpp"
#include <cstring>

HTTP::HTTP():
  ca({
    (void*) HTTP_CA_data,
    HTTP_CA_size,
    CURL_BLOB_NOCOPY
  }),
  count(0)
{
  handler = curl_multi_init();
  curl_multi_setopt(handler, CURLMOPT_MAX_TOTAL_CONNECTIONS, 5);
}

HTTP::~HTTP() {
  curl_multi_cleanup(handler);
}

HTTPRequest* HTTP::request(std::string url, const std::string& body) {
  if (url.find("pastebin://") == 0) {
    url = "https://pastebin.com/raw/" + url.substr(11);
  }
  count++;
  CURL* curl = curl_easy_init();
  HTTPRequest* request = new HTTPRequest(curl, handler);
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
  curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &ca);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, 1024 * 1024 * 10);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
  curl_easy_setopt(curl, CURLOPT_PRIVATE, (void*) request);
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) &request->response);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  if (body.size()) {
    request->headers = curl_slist_append(request->headers, "Content-type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request->headers);
    curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, body.c_str());
  }
  curl_multi_add_handle(handler, curl);
  return request;
}

void HTTP::update() {
  curl_multi_perform(handler, &count);
  int msgs = 0;
  CURLMsg *msg;
  while ((msg = curl_multi_info_read(handler, &msgs))) {
    if (msg->msg == CURLMSG_DONE) {
      HTTPRequest* request;
      curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &request);
      if (msg->data.result == CURLE_OK) {
        curl_easy_getinfo(request->curl, CURLINFO_RESPONSE_CODE, &request->response.status);
      }
      curl_multi_remove_handle(handler, request->curl);
      curl_easy_cleanup(request->curl);
      if (request->headers != nullptr) {
        curl_slist_free_all(request->headers);
        request->headers = nullptr;
      }
      request->curl = nullptr;
      request->handler = nullptr;
      request->isReady = true;
    }
  }
}

bool HTTP::isValidURL(const std::string& url) {
  return (
    url.find("http://") == 0
    || url.find("https://") == 0
    || url.find("pastebin://") == 0
  );
}

size_t HTTP::WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  size_t realsize = size * nmemb;
  HTTPResponse* response = (HTTPResponse*) userp;
 
  unsigned char* ptr = (unsigned char*) realloc(response->data, response->size + realsize + 1);
  if (!ptr) {
    return 0;
  }

  response->data = ptr;
  memcpy(&(response->data[response->size]), contents, realsize);
  response->size += realsize;
  response->data[response->size] = 0;
 
  return realsize;
}

HTTPRequest::HTTPRequest(CURL* curl, CURLM* handler): isReady(false), curl(curl), headers(nullptr), handler(handler) {
  response.data = (unsigned char*) malloc(1);
  response.data[0] = 0;
  response.size = 0;
  response.status = 0;
}

HTTPRequest::~HTTPRequest() {
  if (curl != nullptr && handler != nullptr) {
    curl_multi_remove_handle(handler, curl);
    curl_easy_cleanup(curl);
  }
  if (headers != nullptr) {
    curl_slist_free_all(headers);
  }
  free(response.data);
}
