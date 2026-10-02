cmake_minimum_required(VERSION 3.13)
project(RealHitbox VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK environment variable is not set.")
endif()

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    $ENV{GEODE_SDK}/bindings
)

target_link_libraries(${PROJECT_NAME}
    $ENV{GEODE_SDK}/build/geode.loader.lib
)

setup_geode_mod(${PROJECT_NAME})
