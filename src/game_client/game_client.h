#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <string>
#include <vector>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

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

    GameClient(const std::string &host, int port);
    ~GameClient();

    // 初始化CURL
    ErrorCode initialize();
    // 获取一个样本值，使用输出参数
    ErrorCode getSample(double &value);
    // 提交猜测值，使用输出参数
    ErrorCode submitGuess(double guess, std::string &response);
    // 运行游戏主循环
    void run();

private:
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, std::string *userp);
    // 计算当前样本的均值
    double calculateGuess(const std::vector<double> &samples);

private:
    CURL *m_curl;
    std::string m_host;
    int m_port;
    std::string m_baseUrl;
    std::string m_submitUrl;
};

#endif // GAME_CLIENT_H
