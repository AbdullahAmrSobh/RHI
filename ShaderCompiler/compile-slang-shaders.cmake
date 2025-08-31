function(compile_slang_shaders2)
    # Parse the input arguments
    cmake_parse_arguments(
        SLANG_SHADER
        "" # No boolean flags
        "TARGET;NAME;OUTPUT_DIR" # Single-valued arguments
        "DEPENDENCIES;SHADER_GFX_FILES;SHADER_COMPUTE_FILES;INCLUDE_DIRS" # Multi-valued arguments
        ${ARGN}
    )

    # Ensure required arguments are provided
    if(NOT SLANG_SHADER_TARGET OR NOT SLANG_SHADER_OUTPUT_DIR)
        message(FATAL_ERROR "TARGET and OUTPUT_DIR are required arguments.")
    endif()

    # Prepare include directories
    set(SLANG_SHADER_INCLUDE_FLAGS "")
    foreach(INCLUDE_DIR IN LISTS SLANG_SHADER_INCLUDE_DIRS)
        list(APPEND SLANG_SHADER_INCLUDE_FLAGS "-I${INCLUDE_DIR}")
    endforeach()

    # Use slangc from the Vulkan SDK
    set(SHADER_COMPILER $<TARGET_FILE:ShaderCompiler>)
    set(SPIRV_LINK_EXECUTABLE "$ENV{VULKAN_SDK}/bin/spirv-link${CMAKE_EXECUTABLE_SUFFIX}")

    # Initialize list to store output files
    set(COMPILE_SLANG_SHADERS_OUTPUT_FILES)

    # Function to add a custom command for compiling a shader
    function(add_slang_shader_compile_command SHADER_PATH ENTRY_POINT STAGE_SUFFIX)
        get_filename_component(SHADER_NAME ${SHADER_PATH} NAME_WE)
        set(OUTPUT_PATH "${SLANG_SHADER_OUTPUT_DIR}/${SHADER_NAME}.${STAGE_SUFFIX}.spv")

        add_custom_command(
            OUTPUT ${OUTPUT_PATH}
            COMMAND ${SHADER_COMPILER}
            # -g3 # fixme: WebGPU fails to compile shader module with this flag!
            ${SLANG_SHADER_INCLUDE_FLAGS}
                    -output ${OUTPUT_PATH}
                    -entry ${ENTRY_POINT}
                    -stage ${STAGE_SUFFIX}
                    -target spirv
                    -shader ${SHADER_PATH}
            DEPENDS $<TARGET_FILE:ShaderCompiler> ${SHADER_PATH} ${SLANG_SHADER_DEPENDENCIES}
            COMMENT "Compiling ${STAGE_SUFFIX} shader: ${SHADER_PATH}..."
        )

        # # If both vertex and fragment shaders exist, merge them using spirv-link
        # if(${STAGE_SUFFIX} STREQUAL "fragment")
        #     get_filename_component(SHADER_NAME ${SHADER_PATH} NAME_WE)
        #     set(VERTEX_SPV "${SLANG_SHADER_OUTPUT_DIR}/${SHADER_NAME}.vertex.spv")
        #     set(FRAGMENT_SPV "${SLANG_SHADER_OUTPUT_DIR}/${SHADER_NAME}.fragment.spv")
        #     set(MODULE_SPV "${SLANG_SHADER_OUTPUT_DIR}/${SHADER_NAME}.module.spv")
        #     add_custom_command(
        #     OUTPUT ${MODULE_SPV}
        #     COMMAND ${SPIRV_LINK_EXECUTABLE} -o ${MODULE_SPV} ${VERTEX_SPV} ${FRAGMENT_SPV}
        #     DEPENDS ${SPIRV_LINK_EXECUTABLE} ${VERTEX_SPV} ${FRAGMENT_SPV}
        #     COMMENT "Linking vertex and fragment shaders into module: ${MODULE_SPV}..."
        #     )
        #     list(APPEND COMPILE_SLANG_SHADERS_OUTPUT_FILES ${MODULE_SPV})
        #     set(COMPILE_SLANG_SHADERS_OUTPUT_FILES ${COMPILE_SLANG_SHADERS_OUTPUT_FILES} PARENT_SCOPE)
        # endif()

        list(APPEND COMPILE_SLANG_SHADERS_OUTPUT_FILES ${OUTPUT_PATH})
        set(COMPILE_SLANG_SHADERS_OUTPUT_FILES ${COMPILE_SLANG_SHADERS_OUTPUT_FILES} PARENT_SCOPE)
    endfunction()

    # Iterate over graphics shader files
    if(SLANG_SHADER_SHADER_GFX_FILES)
        foreach(SHADER_FILE IN LISTS SLANG_SHADER_SHADER_GFX_FILES)
            add_slang_shader_compile_command(${SHADER_FILE} "VSMain" "vertex")
            add_slang_shader_compile_command(${SHADER_FILE} "PSMain" "fragment")
        endforeach()
    endif()

    # Iterate over compute shader files
    if(SLANG_SHADER_SHADER_COMPUTE_FILES)
        foreach(SHADER_FILE IN LISTS SLANG_SHADER_SHADER_COMPUTE_FILES)
            add_slang_shader_compile_command(${SHADER_FILE} "CSMain" "compute")
        endforeach()
    endif()

    # Add a custom target to compile all shaders
    add_custom_target(${SLANG_SHADER_TARGET}-compile-shaders
        DEPENDS ${COMPILE_SLANG_SHADERS_OUTPUT_FILES}
    )

    # Ensure the main target depends on the shader compilation target
    add_dependencies(${SLANG_SHADER_TARGET} ${SLANG_SHADER_TARGET}-compile-shaders)
endfunction()
