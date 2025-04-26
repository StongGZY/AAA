#include "curl_pool.h"

CurlPool::CurlPool(size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        CURL *curl = curl_easy_init();
        if (curl)
        {
            pool_.push(curl);
        }
    }
}

CurlPool::~CurlPool()
{
    while (!pool_.empty())
    {
        CURL *curl = pool_.front();
        pool_.pop();
        curl_easy_cleanup(curl);
    }
}

CURL *CurlPool::Acquire()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (!pool_.empty())
    {
        CURL *curl = pool_.front();
        pool_.pop();
        return curl;
    }
    return curl_easy_init();
}

void CurlPool::Release(CURL *handle)
{
    if (!handle)
        return;
    std::unique_lock<std::mutex> lock(mutex_);
    pool_.push(handle);
}