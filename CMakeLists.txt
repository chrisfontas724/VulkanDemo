# Copyright 2019 Chris Fontas. All rights reserved.
# Use of this source code is governed by the license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.21)

project("VulkanDemo"
    VERSION 0.9.1
    DESCRIPTION "A project to test the vulkan wrappers library"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME})

find_package(Windowing CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(UsefulUtils CONFIG REQUIRED)
find_package(FileStreaming CONFIG REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(VulkanWrappers CONFIG REQUIRED)

add_subdirectory(src)
add_subdirectory(demos)

target_sources(${PROJECT_NAME}
    PRIVATE
    main.cpp
    ${SOURCE}
)

# Set the include directories.
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE gfx::VulkanWrappers glm glfw display::Windowing cxl::FileStreaming)

# Copy over the DLLs we need.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                   ${VulkanWrappers_RUNTIME_LIBS}
                   ${CMAKE_CURRENT_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompileShaders)