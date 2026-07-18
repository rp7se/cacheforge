cmake_minimum_required(VERSION 3.16)

project(CacheForge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(cacheforge_core
    src/KVStore.cpp
)

target_include_directories(cacheforge_core
    PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

add_executable(cacheforge_server
    src/main.cpp
)

target_link_libraries(cacheforge_server PRIVATE cacheforge_core)

enable_testing()

add_executable(cacheforge_kvstore_tests
    tests/KVStoreTests.cpp
)

target_link_libraries(cacheforge_kvstore_tests PRIVATE cacheforge_core)

add_test(NAME kvstore_tests COMMAND cacheforge_kvstore_tests)
