cmake_minimum_required(VERSION 3.16)

project(cpp-engine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

option(TSAN "Enable ThreadSanitizer" OFF)

if(TSAN)
    add_compile_options(-fsanitize=thread -g)
    add_link_options(-fsanitize=thread)
endif()

option(ASAN "Enable AddressSanitizer" OFF)

if(ASAN)
    add_compile_options(-fsanitize=address -g)
    add_link_options(-fsanitize=address)
endif()

add_library(engine_lib
    src/kvstore/KVStore.cpp
    src/kvstore/TTLManager.cpp
    src/taskqueue/ThreadPool.cpp
    src/pubsub/EventBus.cpp
    src/persistence/WAL.cpp
    src/engine/Engine.cpp
    src/network/TCPServer.cpp
)

target_include_directories(engine_lib PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(engine main.cpp)

target_link_libraries(engine engine_lib)

add_executable(demo_kvstore demos/demo_kvstore.cpp)
target_link_libraries(demo_kvstore engine_lib)

add_executable(demo_taskqueue demos/demo_taskqueue.cpp)
target_link_libraries(demo_taskqueue engine_lib)

add_executable(demo_pubsub demos/demo_pubsub.cpp)
target_link_libraries(demo_pubsub engine_lib)

add_executable(stress_test tests/stress_test.cpp)
target_link_libraries(stress_test engine_lib)