cmake_minimum_required(VERSION 3.16)
project(BoilerBulk 
    VERSION 1.0.0 
    DESCRIPTION "Personalized meal planner for Purdue University dining courts" 
    LANGUAGES CXX 
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(BoilerBulk
    src/retrieve.cpp
)

target_include_directories(BoilerBulk PRIVATE 
    ${PROJECT_SOURCE_DIR}/include 
)

target_link_libraries(BoilerBulk PRIVATE 
    CURL::libcurl 
    nlohmann_json::nlohmann_json 
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(BoilerBulk PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic 
    )
endif()

