cmake_minimum_required(VERSION 3.16)
project(LidarVisualization VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find packages
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# GLFW - Window management and input
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found, will download and build from source")
    include(FetchContent)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
    )
    FetchContent_MakeAvailable(glfw)
endif()

# GLM - Math library for OpenGL
find_package(glm QUIET)
if(NOT glm_FOUND)
    message(STATUS "GLM not found, will download headers")
    include(FetchContent)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1  # Using newer version compatible with modern CMake
    )
    FetchContent_MakeAvailable(glm)
endif()

# GLAD - OpenGL loader (we'll generate/add this later)
# For now, we'll add a placeholder for GLAD
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glad)
if(EXISTS ${GLAD_DIR})
    add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
    target_include_directories(glad PUBLIC ${GLAD_DIR}/include)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIRS}
)

# Collect source files
file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
)

# Create main executable (placeholder for now)
if(SOURCES)
    add_executable(lidar_viz ${SOURCES})
    
    # Link libraries
    target_link_libraries(lidar_viz
        ${OPENGL_LIBRARIES}
        glfw
        ${CMAKE_THREAD_LIBS_INIT}
    )
    
    if(TARGET glad)
        target_link_libraries(lidar_viz glad)
    endif()
    
    if(TARGET glm::glm)
        target_link_libraries(lidar_viz glm::glm)
    else()
        target_link_libraries(lidar_viz glm)
    endif()
endif()

# Add a test executable for UDP listener (existing test)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_udp_listener.cpp)
    add_executable(test_udp_listener tests/test_udp_listener.cpp)
    target_link_libraries(test_udp_listener ${CMAKE_THREAD_LIBS_INIT})
endif()

# Print configuration summary
message(STATUS "=================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "=================================")
