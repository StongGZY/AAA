#include "game_client/game_client.h"
#include <sstream>

size_t GameClient::writeCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t totalSize = size * nmemb;
    userp->append(static_cast<const char *>(contents), totalSize);
    return totalSize;
}

GameClient::GameClient(const std::string &host, int port)
    : m_host(host), m_port(port), m_curl(nullptr)
{
    std::stringstream ss;
    ss << "http://" << host << ":" << port;
    m_baseUrl = ss.str();
    m_submitUrl = m_baseUrl + "/submit";
}

GameClient::~GameClient()
{
    if (m_curl)
    {
        curl_easy_cleanup(m_curl);
        curl_global_cleanup();
    }
}

GameClient::ErrorCode GameClient::initialize()
{
    curl_global_init(CURL_GLOBAL_ALL);
    m_curl = curl_easy_init();
    return m_curl ? SUCCESS : CURL_INIT_FAILED;
}

GameClient::ErrorCode GameClient::getSample(double &value)
{
    std::string response;

    curl_easy_setopt(m_curl, CURLOPT_URL, m_baseUrl.c_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(m_curl);
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

GameClient::ErrorCode GameClient::submitGuess(double guess, std::string &response)
{
    std::string url = m_submitUrl + "?guess=" + std::to_string(guess);

    curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(m_curl);
    if (res != CURLE_OK)
    {
        std::cerr << "Failed to submit guess: " << curl_easy_strerror(res) << std::endl;
        return CURL_REQUEST_FAILED;
    }

    return SUCCESS;
}

double GameClient::calculateGuess(const std::vector<double> &samples)
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

void GameClient::run()
{
    std::ofstream logFile("game_client.log");
    const int SAMPLES_PER_GUESS = 5; // 每次猜测前获取的样本数

    while (true)
    {
        // 收集多个样本
        std::vector<double> samples;
        bool hasSampleError = false;

        for (int i = 0; i < SAMPLES_PER_GUESS; ++i)
        {
            double sample;
            ErrorCode err = getSample(sample);
            if (err != SUCCESS)
            {
                hasSampleError = true;
                break;
            }
            samples.push_back(sample);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        if (hasSampleError)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // 计算猜测值
        double guess = calculateGuess(samples);

        // 提交猜测并记录结果
        std::string result;
        ErrorCode err = submitGuess(guess, result);
        if (err == SUCCESS)
        {
            logFile << result << std::endl;
            logFile.flush();
        }

        // 等待以满足提交间隔要求
        std::this_thread::sleep_for(std::chrono::milliseconds(900));
    }
}