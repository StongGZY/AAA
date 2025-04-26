#include "game_client.h"
#include "curl_pool/curl_pool.h"
#include <sstream>
#include <mutex>
#include <condition_variable>
#include <thread>

size_t GameClient::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t totalSize = size * nmemb;
    userp->append(static_cast<const char *>(contents), totalSize);
    return totalSize;
}

GameClient::GameClient(const std::string &host, int port, int thread_count, int samples_per_guess)
    : host_(host), port_(port), curl_(nullptr), thread_pool_(new ThreadPool(thread_count)), curl_pool_(new CurlPool(thread_count)), finished_count_(0), samples_per_guess_(samples_per_guess)
{
    std::stringstream ss;
    ss << "http://" << host << ":" << port;
    base_url_ = ss.str();
    submit_url_ = base_url_ + "/submit";
}

GameClient::~GameClient()
{
    if (curl_)
    {
        curl_easy_cleanup(curl_);
        curl_global_cleanup();
    }
}

GameClient::ErrorCode GameClient::Initialize()
{
    curl_global_init(CURL_GLOBAL_ALL);
    curl_ = curl_easy_init();
    return curl_ ? SUCCESS : CURL_INIT_FAILED;
}

GameClient::ErrorCode GameClient::GetSample(double &value)
{
    CURL *curl = curl_pool_->Acquire();
    if (!curl)
    {
        return CURL_INIT_FAILED;
    }

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, base_url_.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_pool_->Release(curl);

    if (res != CURLE_OK)
    {
        std::cerr << "Failed to get sample: " << curl_easy_strerror(res) << std::endl;
        return CURL_REQUEST_FAILED;
    }

    try
    {
        value = std::stod(response);
        return SUCCESS;
    }
    catch (...)
    {
        std::cerr << "Invalid response: " << response << std::endl;
        return INVALID_RESPONSE;
    }
}

GameClient::ErrorCode GameClient::SubmitGuess(double guess, std::string &response)
{
    std::string url = submit_url_ + "?guess=" + std::to_string(guess);

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK)
    {
        std::cerr << "Failed to submit guess: " << curl_easy_strerror(res) << std::endl;
        return CURL_REQUEST_FAILED;
    }

    return SUCCESS;
}

double GameClient::CalculateGuess(const std::vector<double> &samples)
{
    if (samples.empty())
        return 0.0;

    double sum = 0.0;
    for (double sample : samples)
    {
        sum += sample;
    }
    return sum / samples.size();
}

void GameClient::Run()
{
    std::ofstream logFile("game_client.log");

    while (true)
    {
        auto start_time = std::chrono::system_clock::now();

        // 重置计数器和样本容器
        finished_count_ = 0;
        {
            std::lock_guard<std::mutex> lock(samples_mutex_);
            samples_.clear();
            samples_.reserve(samples_per_guess_);
        }

        // 提交采样任务
        for (int i = 0; i < samples_per_guess_; ++i)
        {
            thread_pool_->EnqueueTask([this]
                                      {
                double sample;
                if (GetSample(sample) == SUCCESS)
                {
                    std::lock_guard<std::mutex> lock(samples_mutex_);
                    samples_.push_back(sample);
                }
                ++finished_count_; });
        }

        // 等待所有样本采集完成
        while (finished_count_ < samples_per_guess_)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // 计算猜测值
        double guess;
        {
            std::lock_guard<std::mutex> lock(samples_mutex_);
            guess = CalculateGuess(samples_);
        }

        // 提交猜测并记录结果
        std::string result;
        if (SubmitGuess(guess, result) == SUCCESS)
        {
            logFile << result << std::endl;
            logFile.flush();
        }

        auto end_time = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "Total time: " << duration.count() << "ms, samples: " << samples_.size() << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(900));
    }
}