function(compile_slang_shaders)
	cmake_parse_arguments(
		SLANG_SHADER # prefix of output variables
		"" # list of names of boolean flags (defined ones will be true)
		"TARGET;NAME;OUTPUT_DIR" # single valued arguments
		"DEPENDENCIES;SHADER_GFX_FILES;SHADER_COMPUTE_FILES;INCLUDE_DIRS" # list values arguments
		${ARGN} # the argument that will be parsed
	)

    list(TRANSFORM SLANG_SHADER_INCLUDE_DIRS PREPEND "-I")

    if (WIN32)
        set(SLANGC "${CMAKE_SOURCE_DIR}/slang/windows-x64/release/slangc.exe")
        set(SPIRV_LINK "$ENV{VULKAN_SDK}/bin/spirv-link.exe")
    else()
        message(FATAL_ERROR "not supported platform")
    endif()

	set(COMPILE_SLANG_SHADERS_OUTPUT_FILES)
    foreach(SLANG_SHADER_ABS_PATH ${SLANG_SHADER_SHADER_GFX_FILES})
		get_filename_component(SHADER_NAME ${SLANG_SHADER_ABS_PATH} NAME_WE)

        # compile vertex shader
        set(SLANG_SHADER_OUTPUT_VERTEX_PATH "${SLANG_SHADER_OUTPUT_DIR}/${SHADER_NAME}.vertex.spv")
        add_custom_command(
            OUTPUT ${SLANG_SHADER_OUTPUT_VERTEX_PATH}
            COMMAND ${SLANGC} ${SLANG_SHADER_GFX_PATH} ${SLANG_SHADER_ABS_PATH} ${SLANG_SHADER_INCLUDE_DIRS} -o ${SLANG_SHADER_OUTPUT_VERTEX_PATH} -entry VSMain -stage vertex -target spirv -emit-spirv-directly -fvk-invert-y -fvk-use-entrypoint-name
            DEPENDS ${SLANGC} ${SLANG_SHADER_ABS_PATH} ${SLANG_SHADER_DEPENDENCIES}
            COMMENT "Compiling vertex ${SLANG_SHADER_ABS_PATH}..."
        )

        # compile pixel shader
        set(SLANG_SHADER_OUTPUT_PIXEL_PATH "${SLANG_SHADER_OUTPUT_DIR}/${SHADER_NAME}.pixel.spv")
        add_custom_command(
            OUTPUT ${SLANG_SHADER_OUTPUT_PIXEL_PATH}
            COMMAND ${SLANGC} ${SLANG_SHADER_GFX_PATH} ${SLANG_SHADER_ABS_PATH} ${SLANG_SHADER_INCLUDE_DIRS} -o ${SLANG_SHADER_OUTPUT_PIXEL_PATH} -entry PSMain -stage fragment -target spirv -emit-spirv-directly -fvk-invert-y -fvk-use-entrypoint-name
            DEPENDS ${SLANGC} ${SLANG_SHADER_ABS_PATH} ${SLANG_SHADER_DEPENDENCIES}
            COMMENT "Compiling pixel ${SLANG_SHADER_ABS_PATH}..."
        )

        # generate library
        set(SLANG_SHADER_OUTPUT_MODULE_PATH "${SLANG_SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")
        add_custom_command(
            OUTPUT ${SLANG_SHADER_OUTPUT_MODULE_PATH}
            COMMAND ${SPIRV_LINK} -o ${SLANG_SHADER_OUTPUT_MODULE_PATH} --create-library ${SLANG_SHADER_OUTPUT_VERTEX_PATH} ${SLANG_SHADER_OUTPUT_PIXEL_PATH} --target-env vulkan1.3
            DEPENDS ${SLANG_SHADER_OUTPUT_VERTEX_PATH} ${SLANG_SHADER_OUTPUT_PIXEL_PATH}
            COMMENT "Generating Library ${SLANG_SHADER_OUTPUT_MODULE_PATH}"
        )

		list(APPEND COMPILE_SLANG_SHADERS_OUTPUT_FILES "${SLANG_SHADER_OUTPUT_MODULE_PATH}")
    endforeach(SLANG_SHADER_ABS_PATH)

    add_custom_target(${SLANG_SHADER_TARGET}-compile-shaders DEPENDS ${COMPILE_SLANG_SHADERS_OUTPUT_FILES})
    add_dependencies(${SLANG_SHADER_TARGET} ${SLANG_SHADER_TARGET}-compile-shaders)
endfunction(compile_slang_shaders)
