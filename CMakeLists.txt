# Copyright 2019 Chris Fontas. All rights reserved.
# Use of this source code is governed by the license that can be
# found in the LICENSE file.

cmake_minimum_required (VERSION 3.7)

# Set Project name.
project (VulkanDemo)

# C++ Compiler Settings.
#set(_XOPEN_SOURCE 600)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_FLAGS "-std=gnu++17")



# Create main executable.
add_executable(VulkanDemo main.cpp)

add_subdirectory(libs/VulkanWrappers)
add_subdirectory(libs/VulkanWindowing)

if (NOT TARGET core)
    add_subdirectory(libs/CoreClasses)
endif() 

if (NOT TARGET streaming)
    add_subdirectory(libs/FileSystem)
endif()


# Set the include directories.
set(GLM_INCLUDE_DIR C:/glm)
include_directories(${GLM_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/CoreClasses)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/VulkanWindowing)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/VulkanWrappers)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/FileSystem)
include_directories(${VK_INCLUDE_DIRS})


target_link_libraries(VulkanDemo display glslang vulkan_wrappers)
