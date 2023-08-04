function(CompileShaders ShadersPath, OutputPath)
    find_program(DXC_EXECUTABLE dxc)

    if(NOT DXC_EXECUTABLE)
        message(FATAL_ERROR "dxc compiler not found. Make sure the Vulkan SDK is installed and the dxc executable is in the system's PATH.")
    endif()

    # Get the list of shader source files
    file(GLOB_RECURSE SHADER_SOURCES ${SHADER_PATH}/*.hlsl)

    # Create the output directory for compiled DXIL files
    file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

    foreach(SHADER_SOURCE IN LISTS SHADER_SOURCES)
        # Get the shader source file name without extension
        get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME_WE)

        # Set the output path for the compiled DXIL file
        set(SPRIV_OUTPUT_PATH ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.vert.spriv)

        # Add a custom command to compile the shader
        add_custom_command(
            OUTPUT ${SPRIV_OUTPUT_PATH}
            COMMAND ${DXC_EXECUTABLE} -spirv -T vs_6_1 -E VSMain -Fo ${SPRIV_OUTPUT_PATH} ${SHADER_SOURCE}
            DEPENDS ${SHADER_SOURCE}
            COMMENT "Compiling ${SHADER_SOURCE} to SPRIV"
        )

        # Add the compiled SPRIV file to a list
        list(APPEND SPIRV_OUTPUT_FILES ${SPRIV_OUTPUT_PATH})

        # Set the output path for the compiled DXIL file
        set(SPRIV_PIXEL_OUTPUT_PATH ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.pixel.spriv)

        # Add a custom command to compile the shader
        add_custom_command(
            OUTPUT ${SPRIV_PIXEL_OUTPUT_PATH}
            COMMAND ${DXC_EXECUTABLE} -spirv -T ps_6_1 -E PSMain -Fo ${SPRIV_PIXEL_OUTPUT_PATH} ${SHADER_SOURCE}
            DEPENDS ${SHADER_SOURCE}
            COMMENT "Compiling ${SHADER_SOURCE} to SPRIV"
        )

        # Add the compiled SPRIV file to a list
        list(APPEND SPIRV_OUTPUT_FILES ${SPRIV_PIXEL_OUTPUT_PATH})
    endforeach()
    
    add_custom_target(Shaders DEPENDS ${SPIRV_OUTPUT_FILES})

endfunction(CompileShaders )
