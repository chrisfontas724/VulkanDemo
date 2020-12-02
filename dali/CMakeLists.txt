add_library (dali
    ${CMAKE_CURRENT_SOURCE_DIR}/engines/vk_raytracer.cpp
)


target_include_directories(dali PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
