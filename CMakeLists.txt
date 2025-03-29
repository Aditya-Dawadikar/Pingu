cmake_minimum_required(VERSION 3.14)
project(pingu)

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

include_directories(include)

add_executable(pingu
                main.cpp
                src/json_utils.cpp
                src/http_utils.cpp
                src/test_runner.cpp)
find_package(CURL REQUIRED)

set(CMAKE_BUILD_TYPE Debug)

target_link_libraries(pingu PRIVATE nlohmann_json::nlohmann_json CURL::libcurl)