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
include_directories(${VUlkan_INCLUDE_DIRS})

find_package(UsefulUtils CONFIG REQUIRED)
find_package(FileStreaming CONFIG REQUIRED)

find_package(glfw3 3.3 REQUIRED)

find_package(glm REQUIRED)

find_package(VulkanWrappers CONFIG REQUIRED)
include(GNUInstallDirs)

add_subdirectory(demos)
add_subdirectory(src)

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
    ${SOURCE}
)

# Set the include directories.
include_directories(${INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC gfx::VulkanWrappers PRIVATE glm demo Vulkan::Vulkan glfw display::Windowing)

# Copy over the DLLs we need.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                   ${VulkanWrappers_RUNTIME_LIBS}
                   ${CMAKE_CURRENT_BINARY_DIR}/bin)