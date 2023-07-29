cmake_minimum_required(VERSION 3.15)

project("VulkanDemo"
    VERSION 0.9.1
    DESCRIPTION "A project to test the vulkan wrappers library"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME})

find_package(Windowing CONFIG REQUIRED)

find_package(Vulkan REQUIRED)
include_directories(${VUlkan_INCLUDE_DIRS})

find_package(UsefulUtils CONFIG REQUIRED)
find_package(FileStreaming CONFIG REQUIRED)

find_package(glfw3 3.3 REQUIRED)

find_package(glm REQUIRED)

add_subdirectory(libs/VulkanWrappers)
add_subdirectory(demos)
add_subdirectory(src)


target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
    ${SOURCE}
)

# Set the include directories.
include_directories(${VULKAN_WRAPPER_INCLUDE_DIRS})
include_directories(${INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE glm demo Vulkan::Vulkan glfw display::Windowing)