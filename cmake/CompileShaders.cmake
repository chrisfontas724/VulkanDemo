# Find glslangValidator executable
find_program(GLSLANG_VALIDATOR_EXECUTABLE NAMES glslangValidator)
if(NOT GLSLANG_VALIDATOR_EXECUTABLE)
    message(FATAL_ERROR "glslangValidator not found. Make sure you have Vulkan SDK installed.")
endif()

# Create the output directory for SPIR-V shaders
set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/resources/spirv")
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

# Add custom target for shader compilation
add_custom_target(CompileShaders)

# Define the source directory for shaders
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders")

# Add include directories for shaders
set(SHADER_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/shaders/mwc64x/glsl"
)

# List all the shader files to compile
file(GLOB_RECURSE SHADER_FILES 
    "${SHADER_SOURCE_DIR}/*.vert" 
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/compute/*.comp"
)

# Iterate over shader files and add custom commands to compile them
foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    set(SPIRV_OUTPUT "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")

    # Formulate include flags for glslangValidator
    set(INCLUDE_FLAGS "")
    foreach(INCLUDE_DIR ${SHADER_INCLUDE_DIRS})
        list(APPEND INCLUDE_FLAGS "-I${INCLUDE_DIR}")
    endforeach()

    add_custom_command(
        TARGET CompileShaders
        COMMAND ${GLSLANG_VALIDATOR_EXECUTABLE} ${INCLUDE_FLAGS} -V -o "${SPIRV_OUTPUT}" "${SHADER_FILE}"
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling ${SHADER_NAME} to SPIR-V"
    )
endforeach()

file(COPY
    "${CMAKE_CURRENT_SOURCE_DIR}/data/models/"
    DESTINATION "${CMAKE_BINARY_DIR}/bin/resources/models/"
)

file(COPY
    "${CMAKE_CURRENT_SOURCE_DIR}/data/textures/"
    DESTINATION "${CMAKE_BINARY_DIR}/bin/resources/textures/"
)


# Make sure the top level project is connected to this target.
add_dependencies(${PROJECT_NAME} CompileShaders)