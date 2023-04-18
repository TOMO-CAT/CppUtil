#pragma once

#include <string>
#include <vector>

/**
 * 是否输出debug日志
 */
#ifndef _HTTP_CLIENT_DEBUG
#define _HTTP_CLIENT_DEBUG false
#endif

namespace http_client {

/**
 * @brief HTTPS POST请求
 * @param url 输入参数,请求的Url地址,如:https://www.alipay.com
 * @param post_params 输入参数,使用如下格式param1=val1&param2=val2&…
 * @param timeout_ms 传输超时时间
 * @param conn_timeout_ms 连接超时时间
 * @param resp 输出参数,返回的内容
 * @param ca_path 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
 * @return 返回是否Post成功
 */
int Post(const std::string& url, const std::string& post_params, int timeout_ms, int conn_timeout_ms, std::string* resp,
         const char* ca_path = nullptr);

/**
 * @brief HTTPS GET请求
 * @param url 输入参数,请求的Url地址,如:https://www.alipay.com
 * @param timeout_ms 传输超时时间
 * @param conn_timeout_ms 连接超时时间
 * @param resp 输出参数,返回的内容
 * @param ca_path 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
 * @return 返回是否Post成功
 */
int Get(const std::string& url, int timeout_ms, int conn_timeout_ms, std::string* resp, const char* ca_path = nullptr);

}  // namespace http_client
