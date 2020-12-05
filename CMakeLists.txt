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


add_subdirectory(libs/VulkanWrappers)
add_subdirectory(libs/VulkanWindowing)

if (NOT TARGET core)
    add_subdirectory(libs/CoreClasses)
endif() 

if (NOT TARGET streaming)
    add_subdirectory(libs/FileSystem)
endif()


# Set the include directories.
include_directories(${VULKAN_WRAPPER_INCLUDE_DIRS})


add_subdirectory(src)
add_subdirectory(apps)
