#pragma once

#include <queue>
#include <mutex>
#include <curl/curl.h>

class CurlPool
{
public:
    explicit CurlPool(size_t size);
    ~CurlPool();

    // 禁用拷贝
    CurlPool(const CurlPool &) = delete;
    CurlPool &operator=(const CurlPool &) = delete;

    // 获取一个CURL句柄
    CURL *Acquire();

    // 归还一个CURL句柄
    void Release(CURL *handle);

private:
    std::queue<CURL *> pool_;
    std::mutex mutex_;
};