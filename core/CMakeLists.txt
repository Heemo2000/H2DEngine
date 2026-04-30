cmake_minimum_required(VERSION 3.24)

project(H2DCore VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


find_package(Vulkan REQUIRED)
find_package(ktx CONFIG REQUIRED)

add_subdirectory(third-party/box2d-3.1.1)
add_subdirectory(third-party/glfw)
add_subdirectory(third-party/glm)
add_subdirectory(third-party/miniaudio-0.11.25)
add_subdirectory(third-party/spdlog)
add_subdirectory(third-party/VulkanMemoryAllocator)
add_subdirectory(third-party/yaml-cpp)

file(GLOB_RECURSE ENGINE_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)


# Path to your prebuilt Slang
set(SLANG_ROOT "${CMAKE_SOURCE_DIR}/core/third-party/slang")

add_library(slang SHARED IMPORTED)

set_target_properties(slang PROPERTIES
    IMPORTED_IMPLIB "${SLANG_ROOT}/lib/slang.lib"
    IMPORTED_LOCATION "${SLANG_ROOT}/bin/slang.dll"
)

# Only add include dir IF it exists
if(EXISTS "${SLANG_ROOT}/include")
    target_include_directories(slang INTERFACE
        "${SLANG_ROOT}/include"
    )
endif()

set_target_properties(slang PROPERTIES
                      IMPORTED_LOCATION "${SLANG_ROOT}/bin/slang.dll"
                      IMPORTED_IMPLIB "${SLANG_ROOT}/lib/slang.lib"
                      INTERFACE_INCLUDE_DIRECTORIES "${SLANG_ROOT}/include")


add_library(${PROJECT_NAME} STATIC ${ENGINE_SOURCE_FILES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PREFIX "H2D"
    FILES ${ENGINE_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        box2d
        glfw
        glm::glm
        miniaudio
        spdlog::spdlog
        Vulkan::Vulkan
        yaml-cpp::yaml-cpp
        KTX::ktx
        GPUOpen::VulkanMemoryAllocator
        slang
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"    
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/lib/Debug"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/lib/Release"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SLANG_ROOT}/bin/slang.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)