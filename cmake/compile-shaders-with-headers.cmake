# ==========================================================
# CMake Shader Compilation Script
# Compiles .slang shaders via ShaderCompiler and generates C++ headers.
# ==========================================================

# TODO: This script is vibe coded (I don't like it but it works for now), will cleanup later

# Incremental-safe shader compile helper
function(compile_shaders_with_headers)
    cmake_parse_arguments(
        SHADER
        ""
        "TARGET;OUTPUT_DIR;HEADER_OUTPUT_DIR"
        "SHADER_FILES;INCLUDE_DIRS;ENTRY_POINTS"
        ${ARGN}
    )

    if(NOT SHADER_TARGET OR NOT SHADER_OUTPUT_DIR)
        message(FATAL_ERROR "compile_shaders_with_headers: TARGET and OUTPUT_DIR are required.")
    endif()

    if(NOT SHADER_HEADER_OUTPUT_DIR)
        set(SHADER_HEADER_OUTPUT_DIR "${SHADER_OUTPUT_DIR}/include/shaders")
    endif()

    file(MAKE_DIRECTORY "${SHADER_OUTPUT_DIR}")
    file(MAKE_DIRECTORY "${SHADER_HEADER_OUTPUT_DIR}")

    # Command to run - depend on the target name so CMake builds it when required.
    set(SHADER_COMPILER_TARGET ShaderCompiler)
    set(SHADER_COMPILER_CMD $<TARGET_FILE:${SHADER_COMPILER_TARGET}>)

    find_program(CLANG_FORMAT_EXECUTABLE clang-format)

    set(ALL_SPV)
    set(ALL_HEADERS)

    foreach(SHADER_FILE IN LISTS SHADER_SHADER_FILES)
        get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE)
        # Absolute build-tree paths for outputs
        set(SPIRV_OUT "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spirv")
        set(HEADER_OUT "${SHADER_HEADER_OUTPUT_DIR}/${SHADER_NAME}.hpp")
        # temp header in build tree
        set(HEADER_TMP "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.hpp.tmp")

        # Build include flags
        set(INCLUDE_FLAGS "")
        foreach(INC IN LISTS SHADER_INCLUDE_DIRS)
            list(APPEND INCLUDE_FLAGS "-i" "${INC}")
        endforeach()

        # Entry points (auto-detect if not provided)
        if(SHADER_ENTRY_POINTS)
            set(ENTRY_POINTS ${SHADER_ENTRY_POINTS})
        else()
            file(READ ${SHADER_FILE} SHADER_CONTENT)
            set(ENTRY_POINTS "")
            foreach(PREFIX IN ITEMS VS PS FS CS)
                if(SHADER_CONTENT MATCHES ".*${PREFIX}Main.*")
                    list(APPEND ENTRY_POINTS "${PREFIX}:${PREFIX}Main")
                endif()
            endforeach()
            if(NOT ENTRY_POINTS)
                message(WARNING "No entry points detected in ${SHADER_FILE}. Skipping.")
                continue()
            endif()
            message(STATUS "Auto-detected entry points for ${SHADER_FILE}: ${ENTRY_POINTS}")
        endif()

        set(ENTRY_FLAGS "")
        foreach(ENTRY IN LISTS ENTRY_POINTS)
            list(APPEND ENTRY_FLAGS "-e" "${ENTRY}")
        endforeach()

        #
        # IMPORTANT: write to a temporary header, format it, then only copy_if_different
        # This prevents touching HEADER_OUT when its content didn't change.
        #
        add_custom_command(
            OUTPUT "${SPIRV_OUT}" "${HEADER_OUT}"
            COMMAND ${SHADER_COMPILER_CMD}
                    -s "${SHADER_FILE}"
                    ${ENTRY_FLAGS} ${INCLUDE_FLAGS}
                    -o "${SPIRV_OUT}" -g "${HEADER_TMP}"
            # format the tmp header (if clang-format found)
            COMMAND ${CMAKE_COMMAND} -E echo_append "" # keep command list stable
            COMMAND ${CLANG_FORMAT_EXECUTABLE} -i "${HEADER_TMP}"
            # compare tmp -> final; use copy_if_different so timestamp only changes if content changed
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${HEADER_TMP}" "${HEADER_OUT}"
            # cleanup tmp only if copy_if_different left final identical (copy_if_different won't create a new file when identical)
            COMMAND ${CMAKE_COMMAND} -E rm -f "${HEADER_TMP}"
            DEPENDS "${SHADER_FILE}" ${SHADER_COMPILER_TARGET}
            COMMENT "Compiling shader ${SHADER_FILE} -> ${SPIRV_OUT} + ${HEADER_OUT}"
            VERBATIM
        )

        list(APPEND ALL_SPV "${SPIRV_OUT}")
        list(APPEND ALL_HEADERS "${HEADER_OUT}")
    endforeach()

    # Mark generated artifacts (helps IDEs)
    if(ALL_SPV)
        set_source_files_properties(${ALL_SPV} PROPERTIES GENERATED TRUE)
    endif()
    if(ALL_HEADERS)
        set_source_files_properties(${ALL_HEADERS} PROPERTIES GENERATED TRUE)
    endif()

    # Custom target depends on the real output files (so it won't always run)
    add_custom_target(${SHADER_TARGET}_compile_shaders
        DEPENDS ${ALL_SPV} ${ALL_HEADERS}
    )

    # Make sure the target using shaders depends on the compile step
    add_dependencies(${SHADER_TARGET} ${SHADER_TARGET}_compile_shaders)

    # expose properties
    set_property(TARGET ${SHADER_TARGET} PROPERTY SHADER_OUTPUT_FILES ${ALL_SPV})
    set_property(TARGET ${SHADER_TARGET} PROPERTY SHADER_HEADER_FILES ${ALL_HEADERS})
    set_property(TARGET ${SHADER_TARGET} PROPERTY SHADER_HEADER_DIR ${SHADER_HEADER_OUTPUT_DIR})
    set_property(GLOBAL PROPERTY ${SHADER_TARGET}_SHADER_HEADER_DIR ${SHADER_HEADER_OUTPUT_DIR})
endfunction()
# ==========================================================
# Helper: Add shader headers from another target
# ==========================================================
function(add_shader_headers TARGET_NAME)
    cmake_parse_arguments(
        SHADER_HEADERS
        "PUBLIC;PRIVATE;INTERFACE"
        ""
        "FROM_TARGETS"
        ${ARGN}
    )

    if(NOT SHADER_HEADERS_PUBLIC AND NOT SHADER_HEADERS_PRIVATE AND NOT SHADER_HEADERS_INTERFACE)
        set(SHADER_HEADERS_PRIVATE TRUE)
    endif()

    foreach(SRC_TARGET IN LISTS SHADER_HEADERS_FROM_TARGETS)
        get_property(HEADER_DIR TARGET ${SRC_TARGET} PROPERTY SHADER_HEADER_DIR)
        if(NOT HEADER_DIR)
            message(WARNING "Target ${SRC_TARGET} has no shader header directory property.")
            continue()
        endif()

        if(SHADER_HEADERS_PUBLIC)
            target_include_directories(${TARGET_NAME} PUBLIC ${HEADER_DIR})
        elseif(SHADER_HEADERS_INTERFACE)
            target_include_directories(${TARGET_NAME} INTERFACE ${HEADER_DIR})
        else()
            target_include_directories(${TARGET_NAME} PRIVATE ${HEADER_DIR})
        endif()
    endforeach()
endfunction()


# ==========================================================
# Helper: Get generated shader files
# ==========================================================
function(get_shader_output_files TARGET_NAME OUTPUT_VAR)
    get_property(FILES TARGET ${TARGET_NAME} PROPERTY SHADER_OUTPUT_FILES)
    set(${OUTPUT_VAR} ${FILES} PARENT_SCOPE)
endfunction()

function(get_shader_header_files TARGET_NAME OUTPUT_VAR)
    get_property(FILES TARGET ${TARGET_NAME} PROPERTY SHADER_HEADER_FILES)
    set(${OUTPUT_VAR} ${FILES} PARENT_SCOPE)
endfunction()
