# Copyright 2019 Chris Fontas. All rights reserved.
# Use of this source code is governed by the license that can be
# found in the LICENSE file.

set(LIBRARY_NAME demo)

set(HEADER_LIST)

set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE
 )


# Make an automatic library - will be static or dynamic based on user setting
add_library(demo 
    model.cpp
    shader_resource.cpp
    text_renderer.cpp
    ${HEADER_LIST})

target_include_directories(${LIBRARY_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/${LIBRARY_NAME}>
    $<INSTALL_INTERFACE:include/${LIBRARY_NAME}>
)
target_include_directories(${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(demo PRIVATE gfx::VulkanWrappers PUBLIC display::Windowing cxl::UsefulUtils cxl::FileStreaming)

# All users of this library will need at least C++11
target_compile_features(demo PUBLIC cxx_std_11)