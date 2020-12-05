# Copyright 2019 Chris Fontas. All rights reserved.
# Use of this source code is governed by the license that can be
# found in the LICENSE file.

# Works with 3.11 and tested through 3.19
cmake_minimum_required(VERSION 3.11...3.19)

# Project name and a few useful settings. Other commands can pick up the results
project(
  VulkanDemo
  VERSION 0.1
  DESCRIPTION "Vulkan Demo App"
  LANGUAGES CXX)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # C++ Compiler Settings.
    set (CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS ON)
    set(CMAKE_CXX_FLAGS "-std=gnu++17")
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
endif()

add_subdirectory(libs/VulkanWrappers)
add_subdirectory(libs/VulkanWindowing)

# Set the include directories.
include_directories(${VULKAN_WRAPPER_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(apps)
