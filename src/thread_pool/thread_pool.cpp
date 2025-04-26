#include "thread_pool.h"

ThreadPool::ThreadPool(size_t threads)
    : stop_(false)
{
    workers_.reserve(threads);
    for (size_t i = 0; i < threads; ++i)
    {
        workers_.emplace_back([this]
                              {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cv_.wait(lock, [this] { 
                        return stop_ || !tasks_.empty(); 
                    });
                    
                    if (stop_ && tasks_.empty()) {
                        return;
                    }
                    
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            } });
    }
}

ThreadPool::~ThreadPool()
{
    Stop();
}

void ThreadPool::EnqueueTask(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        tasks_.push(std::move(task));
    }
    cv_.notify_one();
}

void ThreadPool::Stop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all();

    for (auto &worker : workers_)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}