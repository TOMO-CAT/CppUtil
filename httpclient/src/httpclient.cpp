#include <string.h>
#include <stdlib.h>
#include "curl/curl.h"
#include "httpclient.h"
#include "logger.h"

namespace httpclient {

int debug_func(CURL*, curl_infotype itype, char * p_data, size_t size, void *) {
    if (itype == CURLINFO_TEXT) {
        printf("[TEXT]%s\n", p_data);
    } else if (itype == CURLINFO_HEADER_IN) {
        printf("[HEADER_IN]%s\n", p_data);
    } else if (itype == CURLINFO_HEADER_OUT) {
        printf("[HEADER_OUT]%s\n", p_data);
    } else if (itype == CURLINFO_DATA_IN) {
        printf("[DATA_IN]%s\n", p_data);
    } else if (itype == CURLINFO_DATA_OUT) {
        printf("[DATA_OUT]%s\n", p_data);
    }
    return 0;
}

size_t write_callback_func(void* buffer, size_t size, size_t nmemb, void* lpVoid) {
    std::string* str = reinterpret_cast<std::string*>(lpVoid);
    if (NULL == str || NULL == buffer) {
        return -1;
    }
    char* pData = reinterpret_cast<char*>(buffer);
    str->append(pData, size * nmemb);
    return nmemb;
}

int Post(const std::string& url, const std::string& post_params, std::string& resp, const char* ca_path) {
    CURLcode res;
    CURL* curl = curl_easy_init();
    if (nullptr == curl) {
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
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&resp));
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    if (NULL == ca_path) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    } else {
        // 缺省情况就是PEM，所以无需设置，另外支持DER
        // curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path);
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}

int Get(const std::string& url, std::string& resp, const char* ca_path) {
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
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&resp));
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    if (NULL == ca_path) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path);
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}

}  // namespace httpclient