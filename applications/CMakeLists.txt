add_library (applications
    ${CMAKE_CURRENT_SOURCE_DIR}/application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/application_runner.cpp
)

target_link_library(applications dali)
target_include_directories(applications PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

