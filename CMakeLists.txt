
cmake_minimum_required(VERSION 3.13)

project(cdz_bot)

set(WORKER_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/worker.cpp)
set(SOURCE_EXE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
set(PARSER_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.cpp)
set(VK_API_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/VkApi.cpp)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(worker ${WORKER_LIB})
add_library(parser ${PARSER_LIB})
add_library(vk_api ${VK_API_LIB})
add_executable(app ${SOURCE_EXE})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(app parser worker vk_api Threads::Threads ${CURL_LIBRARIES})
