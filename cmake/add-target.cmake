
# @todo: remove target_link_libraries, target_include_directories ...etc

function(tl_add_target)
    set(options STATIC SHARED HEADERONLY EXECUTABLE)
    set(single_value_args NAME NAMESPACE OUTPUT_SUBDIRECTORY OUTPUT_NAME)
    set(multi_value_args SOURCES HEADERS BUILD_DEPENDENCIES RUNTIME_LIBRARIES INCLUDE_DIRECTORIES PUBLIC_INCLUDE_DIRECTORIES COMPILE_DEFINITIONS TARGET_PROPERTIES)

    cmake_parse_arguments(tl_add_target "${options}" "${single_value_args}" "${multi_value_args}" ${ARGN})

    if (NOT tl_add_target_NAME)
        message(FATEL_ERROR "Target must have a name")
    endif()

    if (tl_add_target_HEADERONLY)
        set(tl_add_target_INTERFACE tl_add_target_HEADERONLY)
    endif()

    if (NOT tl_add_target_HEADERONLY)
        if (NOT tl_add_target_SOURCES)
            message(FATAL_ERROR "You must provide a list of source files files for the target")
        endif()
    endif()

    if (tl_add_target_NAMESPACE)
        if (${tl_add_target_NAME} STREQUAL ${tl_add_target_NAMESPACE})
            set(include_dir_name ${tl_add_target_NAME})
            set(macro_names_prefix ${tl_add_target_NAME})
        else()
            set(include_dir_name ${tl_add_target_NAMESPACE}-${tl_add_target_NAME})
            set(macro_names_prefix ${tl_add_target_NAMESPACE}_${tl_add_target_NAME})
        endif()
    else()
        set(include_dir_name ${tl_add_target_NAME})
        set(macro_names_prefix ${tl_add_target_NAME})
        string(TOUPPER ${macro_names_prefix} macro_names_prefix)
    endif()

    if (NOT tl_add_target_EXECUTABLE)

        if(tl_add_target_STATIC)
            set(target_library_type STATIC)
        endif()

        if(tl_add_target_SHARED)
            set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
            set(target_library_type SHARED)
        endif()

        add_library(${tl_add_target_NAME} ${target_library_type} ${tl_add_target_SOURCES} ${tl_add_target_HEADERS})
        add_library(${tl_add_target_NAMESPACE}::${tl_add_target_NAME} ALIAS ${tl_add_target_NAME})

        target_include_directories(
            ${tl_add_target_NAME} ${warning_guard}
            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include/"
            PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/Export>"
            PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source"
        )

        include(GenerateExportHeader)
        generate_export_header(
            ${tl_add_target_NAME}
            EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/Include/${include_dir_name}/Export.hpp
            EXPORT_MACRO_NAME ${macro_names_prefix}_EXPORT
            CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
        )

    else()
        add_executable(${tl_add_target_NAME} ${tl_add_target_SOURCES} ${tl_add_target_HEADERS})
        add_executable(${tl_add_target_NAMESPACE}::${tl_add_target_NAME} ALIAS ${tl_add_target_NAME})
    endif()

    # Detect the current platform
    if (WIN32 OR WIN64)
        set(PLATFORM_NAME ${tl_add_target_NAME}_PLATFORM_WINDOWS)
    elseif (ANDROID)
        set(PLATFORM_NAME ${tl_add_target_NAME}_PLATFORM_ANDROID)
    elseif (APPLE)
        include(TargetConditionals)
        if (TARGET_OS_MAC)
            set(PLATFORM_NAME ${tl_add_target_NAME}_PLATFORM_MACOS)
        elseif (TARGET_OS_IPHONE)
            set(PLATFORM_NAME ${tl_add_target_NAME}_PLATFORM_IOS)
        endif()
    elseif (UNIX AND NOT APPLE)
        set(PLATFORM_NAME ${tl_add_target_NAME}_PLATFORM_LINUX)
    elseif(FREEBSD)
        set(PLATFORM_NAME ${tl_add_target_NAME}_PLATFORM_FREEBSD)
    endif()

    # Detect the system architecture
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SYSTEM_ARCH_NAME ${tl_add_target_NAME}_SYSTEM_ARCHITECTURE_X86_64)
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
        set (SYSTEM_ARCH_NAME ${tl_add_target_NAME}_SYSTEM_ARCHITECTURE_X86_32)
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(SYSTEM_ARCH_NAME ${tl_add_target_NAME}_SYSTEM_ARCHITECTURE_ARM)
    else()
        set(SYSTEM_ARCH_NAME ${tl_add_target_NAME}_SYSTEM_ARCHITECTURE_UNKNOWN)
    endif()

    # Specify target compile definitions
    target_compile_definitions(${tl_add_target_NAME} PUBLIC ${PLATFORM_NAME} ${SYSTEM_ARCH_NAME})

    if (tl_add_target_COMPILE_DEFINITIONS)
        target_compile_definitions(${tl_add_target_NAME} PRIVATE ${tl_add_target_COMPILE_DEFINITIONS})
    endif()

    # Specify target include directories
    if (tl_add_target_INCLUDE_DIRECTORIES)
        target_include_directories(${tl_add_target_NAME} PRIVATE ${tl_add_target_INCLUDE_DIRECTORIES})
    endif()

    # Specify target public include directories
    if (tl_add_target_PUBLIC_INCLUDE_DIRECTORIES)
        target_include_directories(${tl_add_target_NAME} PUBLIC ${tl_add_target_PUBLIC_INCLUDE_DIRECTORIES})
    endif()

    # Specify target custome properties
    if(tl_add_target_TARGET_PROPERTIES)
        set_target_properties(${tl_add_target_NAME} PROPERTIES ${tl_add_target_TARGET_PROPERTIES})
    endif()

    # Specify target link static libraries
    if (tl_add_target_BUILD_DEPENDENCIES)
        target_link_libraries(${tl_add_target_NAME} ${tl_add_target_BUILD_DEPENDENCIES})
    endif()

    # Specify target link dynamic shared libraries
    if (tl_add_target_RUNTIME_LIBRARIES)
        target_link_libraries(${tl_add_target_NAME} ${tl_add_target_RUNTIME_LIBRARIES})
        foreach(runtime_lib ${tl_add_target_RUNTIME_LIBRARIES})
            add_custom_command(TARGET ${tl_add_target_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                    $<TARGET_FILE:${runtime_lib}>
                    $<TARGET_FILE_DIR:${tl_add_target_NAME}>
            )
        endforeach()
    endif()

    # # Enable all warnings as errors for the target
    # target_compile_options(${tl_add_target_NAME} PRIVATE
    #     $<$<CXX_COMPILER_ID:MSVC>:/WX /W4>
    #     $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall -Wextra>
    #     $<$<CXX_COMPILER_ID:Clang>:-Werror -Wall -Wextra>
    # )

endfunction(tl_add_target)
