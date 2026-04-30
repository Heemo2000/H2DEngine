cmake_minimum_required(VERSION 3.24)

project(H2DEngine VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(core)
add_subdirectory(app)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT App)