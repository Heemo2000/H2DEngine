cmake_minimum_required(VERSION 3.24)

project(App VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE H2DCore)