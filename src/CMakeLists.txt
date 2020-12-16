# Copyright 2019 Chris Fontas. All rights reserved.
# Use of this source code is governed by the license that can be
# found in the LICENSE file.

set(LIBRARY_NAME demo)

set(HEADER_LIST 
    "${VulkanDemo_SOURCE_DIR}/include/demo/application_config.hpp" 
    "${VulkanDemo_SOURCE_DIR}/include/demo/application_runner.hpp"
    "${VulkanDemo_SOURCE_DIR}/include/demo/application.hpp")


# Make an automatic library - will be static or dynamic based on user setting
add_library(demo 
    application_runner.cpp
    application.cpp
    model.cpp
    shader_resource.cpp
    vk_raytracer.cpp
    ${HEADER_LIST})

target_include_directories(${LIBRARY_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/${LIBRARY_NAME}>
    $<INSTALL_INTERFACE:include/${LIBRARY_NAME}>
)
target_include_directories(${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(demo PUBLIC vk_wrappers windowing)

# All users of this library will need at least C++11
target_compile_features(demo PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
