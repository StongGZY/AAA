#include "game_client/game_client.h"
#include <sstream>

size_t GameClient::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t totalSize = size * nmemb;
    userp->append(static_cast<const char *>(contents), totalSize);
    return totalSize;
}

GameClient::GameClient(const std::string &host, int port)
    : host_(host), port_(port), curl_(nullptr)
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
    auto start_time = std::chrono::system_clock::now();

    std::string response;

    curl_easy_setopt(curl_, CURLOPT_URL, base_url_.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK)
    {
        std::cerr << "Failed to get sample: " << curl_easy_strerror(res) << std::endl;
        return CURL_REQUEST_FAILED;
    }

    auto end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Get sample time: " << duration.count() << "ms" << std::endl;

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
    auto start_time = std::chrono::system_clock::now();

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

    auto end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Submit guess time: " << duration.count() << "ms" << std::endl;

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
    const int SAMPLES_PER_GUESS = 1; // 每次猜测前获取的样本数

    while (true)
    {
        // 获取当前时间
        auto start_time = std::chrono::system_clock::now();

        // 收集多个样本
        std::vector<double> samples;

        for (int i = 0; i < SAMPLES_PER_GUESS; ++i)
        {
            double sample;
            ErrorCode err = GetSample(sample);
            if (err != SUCCESS)
            {
                continue;
            }
            samples.push_back(sample);
        }

        // 计算猜测值
        double guess = CalculateGuess(samples);

        // 提交猜测并记录结果
        std::string result;
        ErrorCode err = SubmitGuess(guess, result);
        if (err == SUCCESS)
        {
            logFile << result << std::endl;
            logFile.flush();
        }

        // 计算猜测时间
        auto end_time = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "Guess time: " << duration.count() << "ms" << std::endl;

        // 等待以满足提交间隔要求
        std::this_thread::sleep_for(std::chrono::milliseconds(900));
    }
}