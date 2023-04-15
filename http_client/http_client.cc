#include <cstdlib>
#include <cstring>

#include "curl/curl.h"
#include "http_client/http_client.h"
#include "logger/logger.h"

namespace httpclient {

int debug_func(CURL*, curl_infotype itype, char* p_data, size_t size, void*) {
  if (itype == CURLINFO_TEXT) {
    log_info("[TEXT]%s", p_data);
  } else if (itype == CURLINFO_HEADER_IN) {
    log_info("[HEADER_IN]%s", p_data);
  } else if (itype == CURLINFO_HEADER_OUT) {
    log_info("[HEADER_OUT]%s", p_data);
  } else if (itype == CURLINFO_DATA_IN) {
    log_info("[DATA_IN]%s", p_data);
  } else if (itype == CURLINFO_DATA_OUT) {
    log_info("[DATA_OUT]%s", p_data);
  }
  return 0;
}

size_t write_callback_func(void* buffer, size_t size, size_t nmemb, void* p_data) {
  std::string* str = reinterpret_cast<std::string*>(p_data);
  if (nullptr == str || nullptr == buffer) {
    return -1;
  }
  char* p_buffer = reinterpret_cast<char*>(buffer);
  // size always equals to 1
  str->append(p_buffer, size * nmemb);
  return nmemb;
}

int Post(const std::string& url, const std::string& post_params, int timeout_ms, int conn_timeout_ms,
         std::string* const resp, const char* ca_path) {
  CURLcode res;
  CURL* curl = curl_easy_init();
  if (nullptr == curl) {
    log_error("curl_easy_init fail");
    return CURLE_FAILED_INIT;
  }
  if (_HTTP_CLIENT_DEBUG) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_func);
  }
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_params.c_str());
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_func);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(resp));
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  if (nullptr == ca_path) {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
  } else {
    // 缺省情况就是PEM，所以无需设置，另外支持DER
    // curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
    curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path);
  }
  // without set this param, ms timeout is not work
  // http://www.laruence.com/2014/01/21/2939.html
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, conn_timeout_ms);
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    log_error("curl_easy_perform fail, err:%s url:%s", curl_easy_strerror(res), url.c_str());
  }
  curl_easy_cleanup(curl);
  return res;
}

int Get(const std::string& url, int timeout_ms, int conn_timeout_ms, std::string* resp, const char* ca_path) {
  CURLcode res;
  CURL* curl = curl_easy_init();
  if (NULL == curl) {
    return CURLE_FAILED_INIT;
  }
  if (_HTTP_CLIENT_DEBUG) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_func);
  }
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_func);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(resp));
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  if (NULL == ca_path) {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
  } else {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
    curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path);
  }
  // without set this param, ms timeout is not work
  // http://www.laruence.com/2014/01/21/2939.html
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, conn_timeout_ms);
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    log_error("curl_easy_perform fail, err:%s url:%s", curl_easy_strerror(res), url.c_str());
  }
  curl_easy_cleanup(curl);
  return res;
}

}  // namespace httpclient