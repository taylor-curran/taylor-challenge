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

# Compiler flags (base flags only, target-specific warnings added later)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
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

# GLAD - OpenGL loader
include(FetchContent)
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
    CMAKE_ARGS -DCMAKE_POLICY_DEFAULT_CMP0048=NEW
)
# Use a custom approach to avoid CMake version issues with GLAD
FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    FetchContent_Populate(glad)
    # Don't add subdirectory, just build GLAD manually
    add_library(glad_target STATIC ${glad_SOURCE_DIR}/src/glad.c)
    target_include_directories(glad_target PUBLIC ${glad_SOURCE_DIR}/include)
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
    
    # Add target-scoped warnings
    target_compile_options(lidar_viz PRIVATE -Wall -Wextra -Wpedantic)
    
    # Link libraries
    target_link_libraries(lidar_viz
        ${OPENGL_LIBRARIES}
        glfw
        glad_target
        ${CMAKE_THREAD_LIBS_INIT}
    )
    
    if(TARGET glm::glm)
        target_link_libraries(lidar_viz glm::glm)
    else()
        target_link_libraries(lidar_viz glm)
    endif()
    
    # macOS-specific framework linking for vendored GLFW
    if(APPLE)
        target_link_libraries(lidar_viz "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
    endif()
endif()

# Add a test executable for UDP listener (existing test)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_udp_listener.cpp)
    add_executable(test_udp_listener tests/test_udp_listener.cpp)
    target_link_libraries(test_udp_listener ${CMAKE_THREAD_LIBS_INIT})
endif()

# Add test executable for UDP pose receiver
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_pose_receiver.cpp)
    add_executable(test_pose_receiver 
        tests/test_pose_receiver.cpp
        src/udp_receiver.cpp
    )
    target_link_libraries(test_pose_receiver ${CMAKE_THREAD_LIBS_INIT})
endif()

# Print configuration summary
message(STATUS "=================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "=================================")
