cmake_minimum_required(VERSION 3.27)
project(ifmetrica)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_subdirectory(libs/clickhouse-cpp)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(libs/nlohmann_json)

add_executable(${PROJECT_NAME} src/main.cpp
		src/utils.hpp
		src/ifacestate.hpp
		src/config.hpp)

target_include_directories(${PROJECT_NAME} PRIVATE libs/clickhouse-cpp/ libs/clickhouse-cpp/contrib/absl)

target_link_libraries(${PROJECT_NAME} PRIVATE clickhouse-cpp-lib)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)