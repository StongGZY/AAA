#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <string>
#include <vector>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>
#include <memory>
#include "curl_pool/curl_pool.h"
#include "thread_pool/thread_pool.h"

class GameClient
{
public:
    // 错误码定义
    enum ErrorCode
    {
        SUCCESS = 0,
        CURL_INIT_FAILED,
        CURL_REQUEST_FAILED,
        INVALID_RESPONSE
    };

    GameClient(const std::string &host, int port, int thread_count = 10, int samples_per_guess = 10);
    ~GameClient();

    // 初始化CURL
    ErrorCode Initialize();
    // 获取一个样本值，使用输出参数
    ErrorCode GetSample(double &value);
    // 提交猜测值，使用输出参数
    ErrorCode SubmitGuess(double guess, std::string &response);
    // 运行游戏主循环
    void Run();

private:
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);
    // 计算当前样本的均值
    double CalculateGuess(const std::vector<double> &samples);

private:
    // 基本配置
    std::string host_;
    int port_;
    std::string base_url_;
    std::string submit_url_;
    CURL *curl_; // 主CURL句柄，用于提交猜测
    int samples_per_guess_;

    // 线程池和CURL池
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unique_ptr<CurlPool> curl_pool_;

    // 采样结果
    std::vector<double> samples_;
    std::mutex samples_mutex_;
    std::atomic<int> finished_count_;
};

#endif // GAME_CLIENT_H
