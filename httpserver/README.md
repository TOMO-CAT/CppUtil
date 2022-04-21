# HttpServer

## 简介
基于epoll的C++简易Http服务器。

## 特性
* 支持Post和Get请求
* 基于Epoll的IO复用
* 完善的日志输出
* 接口简单

## 设计方案

## EpollSocket: 基于Epoll的服务端Socket

TCP服务端往往需要绑定一个Socket作为服务端监听端口