#pragma once

#include <vector>
#include <string>
#include <boost/noncopyable.hpp>

// TODO: 1. 支持超时 2. 支持重试 3. 支持debug日志

/**
 * 是否输出debug日志
 */
#ifndef _HTTP_CLIENT_DEBUG
#define _HTTP_CLIENT_DEBUG false
#endif
namespace httpclient {

/** 
* @brief HTTPS POST请求
* @param url 输入参数,请求的Url地址,如:https://www.alipay.com 
* @param post_params 输入参数,使用如下格式param1=val1&param2=val2&… 
* @param resp 输出参数,返回的内容 
* @param ca_path 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性. 
* @return 返回是否Post成功 
*/  
int Post(const std::string& url, const std::string& post_params, std::string& resp, const char* ca_path = NULL);

/** 
* @brief HTTPS GET请求
* @param url 输入参数,请求的Url地址,如:https://www.alipay.com 
* @param resp 输出参数,返回的内容 
* @param ca_path 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性. 
* @return 返回是否Post成功 
*/  
int Get(const std::string& url, std::string& resp, const char* ca_path = NULL);

}  // namespace httpclient