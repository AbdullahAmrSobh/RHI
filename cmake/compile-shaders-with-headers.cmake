# CMake script for compiling shaders with custom ShaderCompiler and generating C++ headers
# This script uses the custom RHIShaderCompiler to compile .slang shaders and generate C++ header files

function(compile_shaders_with_headers)
    # Parse the input arguments
    cmake_parse_arguments(
        SHADER_COMPILE
        "" # No boolean flags
        "TARGET;OUTPUT_DIR;HEADER_OUTPUT_DIR" # Single-valued arguments
        "SHADER_FILES;INCLUDE_DIRS;ENTRY_POINTS" # Multi-valued arguments
        ${ARGN}
    )

    # Ensure required arguments are provided
    if(NOT SHADER_COMPILE_TARGET OR NOT SHADER_COMPILE_OUTPUT_DIR)
        message(FATAL_ERROR "TARGET and OUTPUT_DIR are required arguments.")
    endif()

    # Set default header output directory if not provided
    if(NOT SHADER_COMPILE_HEADER_OUTPUT_DIR)
        set(SHADER_COMPILE_HEADER_OUTPUT_DIR "${SHADER_COMPILE_OUTPUT_DIR}/include/shaders")
    endif()

    # Get the ShaderCompiler executable path
    set(SHADER_COMPILER_EXECUTABLE $<TARGET_FILE:ShaderCompiler>)

    # Create output directories
    file(MAKE_DIRECTORY ${SHADER_COMPILE_OUTPUT_DIR})
    file(MAKE_DIRECTORY ${SHADER_COMPILE_HEADER_OUTPUT_DIR})

    # Initialize lists to store output files
    set(COMPILE_SHADERS_OUTPUT_FILES)
    set(COMPILE_SHADERS_HEADER_FILES)
    set(COMPILE_SHADERS_INPUTS)

    find_program(CLANG_FORMAT_EXECUTABLE clang-format)

    # Function to add a custom command for compiling a shader with header generation
    function(add_shader_compile_command SHADER_PATH ENTRY_POINTS_LIST)
        get_filename_component(SHADER_NAME ${SHADER_PATH} NAME_WE)
        set(SPIRV_OUTPUT_PATH "${SHADER_COMPILE_OUTPUT_DIR}/${SHADER_NAME}.spirv")
        set(HEADER_OUTPUT_PATH "${SHADER_COMPILE_HEADER_OUTPUT_DIR}/${SHADER_NAME}.hpp")

        # Build include flags
        set(INCLUDE_FLAGS "")
        foreach(INCLUDE_DIR IN LISTS SHADER_COMPILE_INCLUDE_DIRS)
            list(APPEND INCLUDE_FLAGS "-i" "${INCLUDE_DIR}")
        endforeach()

        # Build entry point flags
        set(ENTRY_FLAGS "")
        foreach(ENTRY_POINT IN LISTS ENTRY_POINTS_LIST)
            list(APPEND ENTRY_FLAGS "-e" "${ENTRY_POINT}")
        endforeach()

        add_custom_command(
            OUTPUT ${SPIRV_OUTPUT_PATH} # ${HEADER_OUTPUT_PATH}
            COMMAND ${SHADER_COMPILER_EXECUTABLE}
                    -s ${SHADER_PATH}
                    ${ENTRY_FLAGS}
                    ${INCLUDE_FLAGS}
                    -o ${SPIRV_OUTPUT_PATH}
                    -g ${HEADER_OUTPUT_PATH}
            COMMAND
                    ${CLANG_FORMAT_EXECUTABLE} -i ${HEADER_OUTPUT_PATH}
            DEPENDS ShaderCompiler ${SHADER_PATH}
            COMMENT "Compiling shader: ${SHADER_PATH}..."
            VERBATIM
        )

        list(APPEND COMPILE_SHADERS_OUTPUT_FILES ${SPIRV_OUTPUT_PATH})
        list(APPEND COMPILE_SHADERS_HEADER_FILES ${HEADER_OUTPUT_PATH})
        list(APPEND COMPILE_SHADERS_INPUTS ${SHADER_PATH})
        set(COMPILE_SHADERS_OUTPUT_FILES ${COMPILE_SHADERS_OUTPUT_FILES} PARENT_SCOPE)
        set(COMPILE_SHADERS_HEADER_FILES ${COMPILE_SHADERS_HEADER_FILES} PARENT_SCOPE)
        set(COMPILE_SHADERS_INPUTS ${SHADER_PATH} PARENT_SCOPE)
    endfunction()

    # Process shader files
    if(SHADER_COMPILE_SHADER_FILES)
        foreach(SHADER_FILE IN LISTS SHADER_COMPILE_SHADER_FILES)
            # Check if specific entry points are provided for this shader
            set(SHADER_ENTRY_POINTS ${SHADER_COMPILE_ENTRY_POINTS})

            # If no entry points specified, try to detect common patterns
            if(NOT SHADER_ENTRY_POINTS)
                # Look for common entry point patterns in the shader file
                file(READ ${SHADER_FILE} SHADER_CONTENT)

                set(DETECTED_ENTRY_POINTS "")
                if(SHADER_CONTENT MATCHES ".*VSMain.*")
                    list(APPEND DETECTED_ENTRY_POINTS "VS:VSMain")
                endif()
                if(SHADER_CONTENT MATCHES ".*PSMain.*")
                    list(APPEND DETECTED_ENTRY_POINTS "PS:PSMain")
                endif()
                if(SHADER_CONTENT MATCHES ".*FSMain.*")
                    list(APPEND DETECTED_ENTRY_POINTS "FS:FSMain")
                endif()
                if(SHADER_CONTENT MATCHES ".*CSMain.*")
                    list(APPEND DETECTED_ENTRY_POINTS "CS:CSMain")
                endif()

                if(DETECTED_ENTRY_POINTS)
                    set(SHADER_ENTRY_POINTS ${DETECTED_ENTRY_POINTS})
                    message(STATUS "Auto-detected entry points for ${SHADER_FILE}: ${SHADER_ENTRY_POINTS}")
                else()
                    message(WARNING "No entry points detected for ${SHADER_FILE}. Please specify ENTRY_POINTS manually.")
                    continue()
                endif()
            endif()

            add_shader_compile_command(${SHADER_FILE} "${SHADER_ENTRY_POINTS}")
        endforeach()
    endif()

    # Add a custom target to compile all shaders
    add_custom_target(${SHADER_COMPILE_TARGET}-compile-shaders
        DEPENDS ${COMPILE_SHADERS_OUTPUT_FILES} # ${COMPILE_SHADERS_HEADER_FILES}
    )

    # Ensure the main target depends on the shader compilation target
    add_dependencies(${SHADER_COMPILE_TARGET} ${SHADER_COMPILE_TARGET}-compile-shaders)

    # Disabled because, to include a sshader you will have to type #include "shader-name.hpp" instead of "shaders-dir/shader-name"
    # # Add the generated header directory to the target's include directories
    # target_include_directories(${SHADER_COMPILE_TARGET} PUBLIC ${SHADER_COMPILE_HEADER_OUTPUT_DIR})

    # Store the generated files for potential use by other targets
    set_property(TARGET ${SHADER_COMPILE_TARGET} PROPERTY SHADER_OUTPUT_FILES ${COMPILE_SHADERS_OUTPUT_FILES})
    set_property(TARGET ${SHADER_COMPILE_TARGET} PROPERTY SHADER_HEADER_FILES ${COMPILE_SHADERS_HEADER_FILES})
    set_property(TARGET ${SHADER_COMPILE_TARGET} PROPERTY SHADER_HEADER_DIR ${SHADER_COMPILE_HEADER_OUTPUT_DIR})

    # Make the header directory available to other targets that might need it
    set_property(GLOBAL PROPERTY ${SHADER_COMPILE_TARGET}_SHADER_HEADER_DIR ${SHADER_COMPILE_HEADER_OUTPUT_DIR})
endfunction()

# Function to add shader header include directory to a target
function(add_shader_headers TARGET_NAME)
    cmake_parse_arguments(
        SHADER_HEADERS
        "PUBLIC;PRIVATE;INTERFACE" # Boolean flags
        "" # Single-valued arguments
        "FROM_TARGETS" # Multi-valued arguments
        ${ARGN}
    )

    # If no scope specified, default to PRIVATE
    if(NOT SHADER_HEADERS_PUBLIC AND NOT SHADER_HEADERS_PRIVATE AND NOT SHADER_HEADERS_INTERFACE)
        set(SHADER_HEADERS_PRIVATE TRUE)
    endif()

    # Add include directories from specified targets
    foreach(FROM_TARGET IN LISTS SHADER_HEADERS_FROM_TARGETS)
        get_property(HEADER_DIR TARGET ${FROM_TARGET} PROPERTY SHADER_HEADER_DIR)
        if(HEADER_DIR)
            if(SHADER_HEADERS_PUBLIC)
                target_include_directories(${TARGET_NAME} PUBLIC ${HEADER_DIR})
            elseif(SHADER_HEADERS_PRIVATE)
                target_include_directories(${TARGET_NAME} PRIVATE ${HEADER_DIR})
            elseif(SHADER_HEADERS_INTERFACE)
                target_include_directories(${TARGET_NAME} INTERFACE ${HEADER_DIR})
            endif()
        else()
            message(WARNING "Target ${FROM_TARGET} does not have shader header directory property")
        endif()
    endforeach()
endfunction()

# Function to get shader output files from a target
function(get_shader_output_files TARGET_NAME OUTPUT_VARIABLE)
    get_property(SHADER_FILES TARGET ${TARGET_NAME} PROPERTY SHADER_OUTPUT_FILES)
    set(${OUTPUT_VARIABLE} ${SHADER_FILES} PARENT_SCOPE)
endfunction()

# Function to get shader header files from a target
function(get_shader_header_files TARGET_NAME OUTPUT_VARIABLE)
    get_property(HEADER_FILES TARGET ${TARGET_NAME} PROPERTY SHADER_HEADER_FILES)
    set(${OUTPUT_VARIABLE} ${HEADER_FILES} PARENT_SCOPE)
endfunction()
