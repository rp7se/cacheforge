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
    src/CommandParser.cpp
    src/CommandProcessor.cpp
    src/KVStore.cpp
)

target_include_directories(cacheforge_core
    PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

add_executable(cacheforge_server
    src/main.cpp
    src/TcpServer.cpp
)

target_link_libraries(cacheforge_server PRIVATE cacheforge_core)

if(WIN32)
    target_link_libraries(cacheforge_server PRIVATE ws2_32)
endif()

enable_testing()

add_executable(cacheforge_kvstore_tests
    tests/KVStoreTests.cpp
)

target_link_libraries(cacheforge_kvstore_tests PRIVATE cacheforge_core)

add_test(NAME kvstore_tests COMMAND cacheforge_kvstore_tests)

add_executable(cacheforge_command_parser_tests
    tests/CommandParserTests.cpp
)

target_link_libraries(cacheforge_command_parser_tests PRIVATE cacheforge_core)

add_test(NAME command_parser_tests COMMAND cacheforge_command_parser_tests)

add_executable(cacheforge_command_processor_tests
    tests/CommandProcessorTests.cpp
)

target_link_libraries(cacheforge_command_processor_tests PRIVATE cacheforge_core)

add_test(NAME command_processor_tests COMMAND cacheforge_command_processor_tests)
