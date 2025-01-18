function(compile_slang_shaders)
    # Parse the input arguments
    cmake_parse_arguments(
        SLANG_SHADER
        "" # No boolean flags
        "TARGET;NAME;OUTPUT_DIR" # Single-valued arguments
        "DEPENDENCIES;SHADER_GFX_FILES;SHADER_COMPUTE_FILES;INCLUDE_DIRS" # Multi-valued arguments
        ${ARGN}
    )

    # Ensure required arguments are provided
    if(NOT SLANG_SHADER_TARGET OR NOT SLANG_SHADER_OUTPUT_DIR OR NOT SLANG_SHADER_SHADER_GFX_FILES)
        message(FATAL_ERROR "TARGET, OUTPUT_DIR, and SHADER_GFX_FILES are required arguments.")
    endif()

    # Prepare include directories
    set(SLANG_SHADER_INCLUDE_FLAGS "")
    foreach(INCLUDE_DIR IN LISTS SLANG_SHADER_INCLUDE_DIRS)
        list(APPEND SLANG_SHADER_INCLUDE_FLAGS "-I${INCLUDE_DIR}")
    endforeach()

    # Determine platform-specific executables
    if(WIN32)
        set(SLANGC_EXECUTABLE "${CMAKE_SOURCE_DIR}/Examples/Dependencies/slang-v2024.10/bin/slangc.exe")
        set(SPIRV_LINK_EXECUTABLE "$ENV{VULKAN_SDK}/bin/spirv-link.exe")
    else()
        message(FATAL_ERROR "Platform not supported.")
    endif()

    # Initialize list to store output files
    set(COMPILE_SLANG_SHADERS_OUTPUT_FILES)

    # Function to add a custom command for compiling a shader
    function(add_slang_shader_compile_command SHADER_PATH ENTRY_POINT STAGE_SUFFIX)
        get_filename_component(SHADER_NAME ${SHADER_PATH} NAME_WE)
        set(OUTPUT_PATH "${SLANG_SHADER_OUTPUT_DIR}/${SHADER_NAME}.${STAGE_SUFFIX}.spv")

        add_custom_command(
            OUTPUT ${OUTPUT_PATH}
            COMMAND ${SLANGC_EXECUTABLE}
                    -g3
                    ${SLANG_SHADER_INCLUDE_FLAGS}
                    -o ${OUTPUT_PATH}
                    -matrix-layout-row-major
                    -entry ${ENTRY_POINT}
                    -stage ${STAGE_SUFFIX}
                    -target spirv
                    -emit-spirv-directly
                    -fvk-use-entrypoint-name
                    ${SHADER_PATH}
            DEPENDS ${SLANGC_EXECUTABLE} ${SHADER_PATH} ${SLANG_SHADER_DEPENDENCIES}
            COMMENT "Compiling ${STAGE_SUFFIX} shader: ${SHADER_PATH}..."
        )

        list(APPEND COMPILE_SLANG_SHADERS_OUTPUT_FILES ${OUTPUT_PATH})
        set(COMPILE_SLANG_SHADERS_OUTPUT_FILES ${COMPILE_SLANG_SHADERS_OUTPUT_FILES} PARENT_SCOPE)
    endfunction()

    # Iterate over each shader file and create compile commands
    foreach(SHADER_FILE IN LISTS SLANG_SHADER_SHADER_GFX_FILES)
        add_slang_shader_compile_command(${SHADER_FILE} "VSMain" "vertex")
        add_slang_shader_compile_command(${SHADER_FILE} "PSMain" "fragment")
    endforeach()

    # Add a custom target to compile all shaders
    add_custom_target(${SLANG_SHADER_TARGET}-compile-shaders
        DEPENDS ${COMPILE_SLANG_SHADERS_OUTPUT_FILES}
    )

    # Ensure the main target depends on the shader compilation target
    add_dependencies(${SLANG_SHADER_TARGET} ${SLANG_SHADER_TARGET}-compile-shaders)
endfunction()
