include(CMakeParseArguments)

function(RHICompileShaders)
	cmake_parse_arguments(
		RHI_ARG # prefix of output variables
		"" # list of names of boolean flags (defined ones will be true)
		"TARGET;NAME;OUTPUT_DIR" # single valued arguments
		"DEPENDENCIES;SHADER_FILES;BLOB_FILES" # list values arguments
		${ARGN} # the argument that will be parsed
	)

	set(RHI_OUTPUT_SHADER_FILES)
	foreach(RHI_SHADER_ABS_PATH ${RHI_ARG_SHADER_FILES})
		get_filename_component(RHI_SHADER_NAME ${RHI_SHADER_ABS_PATH} NAME_WE)
		get_filename_component(RHI_SHADER_DIR ${RHI_SHADER_ABS_PATH} DIRECTORY)

		# make sure the compiled shaders folders exists before actually compiling them
		file(MAKE_DIRECTORY ${RHI_ARG_OUTPUT_DIR})

		set(RHI_OUTPUT_VERT_SPV_SHADER_NAME "${RHI_ARG_OUTPUT_DIR}/${RHI_SHADER_NAME}.vertex.spv")
		set(RHI_OUTPUT_PIXEL_SPV_SHADER_NAME "${RHI_ARG_OUTPUT_DIR}/${RHI_SHADER_NAME}.pixel.spv")
		set(RHI_OUTPUT_SPV_SHADER_MODULE_NAME "${RHI_ARG_OUTPUT_DIR}/${RHI_SHADER_NAME}.spv")

        # compile vertex stage
		add_custom_command(
			OUTPUT ${RHI_OUTPUT_VERT_SPV_SHADER_NAME}
			COMMAND $ENV{VULKAN_SDK}/bin/dxc.exe -spirv ${RHI_SHADER_ABS_PATH} -fspv-target-env=vulkan1.3 -E VSMain -T vs_6_0 -Fo ${RHI_OUTPUT_VERT_SPV_SHADER_NAME}
			DEPENDS ${RHI_SHADER_ABS_PATH} ${RHI_ARG_DEPENDENCIES}
			COMMENT "Compiling ${RHI_SHADER_NAME} vertex stage"
		)

        # compile pixel stage
		add_custom_command(
			OUTPUT ${RHI_OUTPUT_PIXEL_SPV_SHADER_NAME}
			COMMAND $ENV{VULKAN_SDK}/bin/dxc.exe -spirv ${RHI_SHADER_ABS_PATH} -fspv-target-env=vulkan1.3 -E PSMain -T ps_6_0 -Fo ${RHI_OUTPUT_PIXEL_SPV_SHADER_NAME}
			DEPENDS ${RHI_SHADER_ABS_PATH} ${RHI_ARG_DEPENDENCIES}
			COMMENT "Compiling ${RHI_SHADER_NAME} pixel stage"
		)

        # generate library
		add_custom_command(
			OUTPUT ${RHI_OUTPUT_SPV_SHADER_MODULE_NAME}
			COMMAND $ENV{VULKAN_SDK}/bin/spirv-link.exe -o ${RHI_OUTPUT_SPV_SHADER_MODULE_NAME} --create-library ${RHI_OUTPUT_VERT_SPV_SHADER_NAME} ${RHI_OUTPUT_PIXEL_SPV_SHADER_NAME}
			DEPENDS ${RHI_OUTPUT_VERT_SPV_SHADER_NAME} ${RHI_OUTPUT_PIXEL_SPV_SHADER_NAME}
			COMMENT "Generating Library  ${RHI_SHADER_ABS_PATH}"
		)

		list(APPEND RHI_OUTPUT_SHADER_FILES "${RHI_OUTPUT_SPV_SHADER_MODULE_NAME}")
	endforeach(RHI_SHADER_ABS_PATH)

	if (RHI_OUTPUT_SHADER_FILES)
		add_custom_target(${RHI_ARG_TARGET}-compile-shaders DEPENDS ${RHI_OUTPUT_SHADER_FILES})
		add_dependencies(${RHI_ARG_TARGET} ${RHI_ARG_TARGET}-compile-shaders)
	endif()
endfunction(RHICompileShaders)
