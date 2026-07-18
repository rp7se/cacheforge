cmake_minimum_required(VERSION 3.16)

project(CacheForge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(cacheforge_server
    src/main.cpp
)

if(MSVC)
    target_compile_options(cacheforge_server PRIVATE /W4)
else()
    target_compile_options(cacheforge_server PRIVATE -Wall -Wextra -Wpedantic)
endif()
