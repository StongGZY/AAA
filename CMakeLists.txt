cmake_minimum_required(VERSION 3.10)
project(network_game_client)

# 设置默认构建类型为Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 设置C++标准为11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置curl的路径
set(CURL_ROOT ${CMAKE_SOURCE_DIR}/deps/curl)
set(CURL_INCLUDE_DIR ${CURL_ROOT}/include)
set(CURL_LIBRARY ${CURL_ROOT}/lib/libcurl.so)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CURL_INCLUDE_DIR})

# 根据构建类型设置编译选项
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# 添加可执行文件
add_executable(game_client
    src/main.cpp
    src/game_client/game_client.cpp
    src/thread_pool/thread_pool.cpp
    src/curl_pool/curl_pool.cpp
)

# 链接curl库
target_link_libraries(game_client PRIVATE ${CURL_LIBRARY}
    pthread # 添加pthread库，因为我们使用了std::thread
)