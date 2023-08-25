
function(aams_add_target)
    set(options STATIC SHARED HEADERONLY EXECUTABLE)
    set(single_value_args NAME NAMESPACE OUTPUT_SUBDIRECTORY OUTPUT_NAME)
    set(multi_value_args SOURCES HEADERS BUILD_DEPENDENCIES RUNTIME_LIBRARIES INCLUDE_DIRECTORIES COMPILE_DEFINITIONS TARGET_PROPERTIES)

    cmake_parse_arguments(aams_add_target "${options}" "${single_value_args}" "${multi_value_args}" ${ARGN})

    # Since the term "INTERFACE" used in  other context such as HEADERS, COMPILE_DEFINITIONS to specifiy property visibility
    # It needs to be parsed after those arguments have been parsed to avoid the usage of INTERFACE as a visibility scope
    cmake_parse_arguments(ly_add_target "INTERFACE" "" "" ${aams_add_target_UNPARSED_ARGUMENTS})

    if (NOT aams_add_target_NAME)
        message(FATEL_ERROR "Target must have a name")
    endif()

    if (aams_add_target_HEADERONLY)
        set(aams_add_target_INTERFACE aams_add_target_HEADERONLY)
    endif()

    if (NOT aams_add_target_HEADERONLY)
        if (NOT aams_add_target_SOURCES)
            message(FATAL_ERROR "You must provide a list of source files files for the target")
        endif()
    endif()

    if (aams_add_target_NAMESPACE)
        if (${aams_add_target_NAME} STREQUAL ${aams_add_target_NAMESPACE})
            set(include_dir_name ${aams_add_target_NAME})
            set(macro_names_prefix ${aams_add_target_NAME})
        else()
            set(include_dir_name ${aams_add_target_NAMESPACE}-${aams_add_target_NAME})
            set(macro_names_prefix ${aams_add_target_NAMESPACE}_${aams_add_target_NAME})
        endif()
    else()
        set(include_dir_name ${aams_add_target_NAME})
        set(macro_names_prefix ${aams_add_target_NAME})
        string(TOUPPER ${macro_names_prefix} macro_names_prefix)
    endif()

    if (NOT aams_add_target_EXECUTABLE)

        if(aams_add_target_STATIC)
            set(target_library_type STATIC)
        endif()

        if(aams_add_target_SHARED)
            set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
            set(target_library_type SHARED)
        endif()

        add_library(${aams_add_target_NAME} ${target_library_type} ${aams_add_target_SOURCES})
        add_library(${aams_add_target_NAMESPACE}::${aams_add_target_NAME} ALIAS ${aams_add_target_NAME})

        if (aams_add_target_STATIC OR aams_add_target_SHARED)
            target_include_directories(
                ${aams_add_target_NAME} ${warning_guard}
                PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include/"
                PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/Export>"
                PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source"
            )
        endif()

        include(GenerateExportHeader)
        generate_export_header(
            ${aams_add_target_NAME}
            EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/Include/${include_dir_name}/Export.hpp
            EXPORT_MACRO_NAME ${macro_names_prefix}_EXPORT
            CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
        )

    else()
        add_executable(${aams_add_target_NAME} ${aams_add_target_SOURCES})
        add_executable(${aams_add_target_NAMESPACE}::${aams_add_target_NAME} ALIAS ${aams_add_target_NAME})
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${aams_add_target_NAME} PRIVATE -fno-exceptions)
        target_compile_options(${aams_add_target_NAME} PRIVATE -fno-rtti)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${aams_add_target_NAME} PRIVATE /GR-)
        target_compile_options(${aams_add_target_NAME} PRIVATE /EHs-c-)
    endif()

    # Detect the current platform
    if (WIN32 OR WIN64)
        set(PLATFORM_NAME ${aams_add_target_NAME}_PLATFORM_WINDOWS)
    elseif (ANDROID)
        set(PLATFORM_NAME ${aams_add_target_NAME}_PLATFORM_ANDROID)
    elseif (APPLE)
        include(TargetConditionals)
        if (TARGET_OS_MAC)
            set(PLATFORM_NAME ${aams_add_target_NAME}_PLATFORM_MACOS)
        elseif (TARGET_OS_IPHONE)
            set(PLATFORM_NAME ${aams_add_target_NAME}_PLATFORM_IOS)
        endif()
    elseif (UNIX AND NOT APPLE)
        set(PLATFORM_NAME ${aams_add_target_NAME}_PLATFORM_LINUX)
    elseif(FREEBSD)
        set(PLATFORM_NAME ${aams_add_target_NAME}_PLATFORM_FREEBSD)
    endif()

    # Detect the system architecture
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SYSTEM_ARCH_NAME ${aams_add_target_NAME}_SYSTEM_ARCHITECTURE_X86_64)
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
        set (SYSTEM_ARCH_NAME ${aams_add_target_NAME}_SYSTEM_ARCHITECTURE_X86_32)
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(SYSTEM_ARCH_NAME ${aams_add_target_NAME}_SYSTEM_ARCHITECTURE_ARM)
    else()
        set(SYSTEM_ARCH_NAME ${aams_add_target_NAME}_SYSTEM_ARCHITECTURE_UNKNOWN)
    endif()

    target_compile_definitions(${aams_add_target_NAME} PUBLIC ${PLATFORM_NAME})
    target_compile_definitions(${aams_add_target_NAME} PUBLIC ${SYSTEM_ARCH_NAME})

    if (aams_add_target_INCLUDE_DIRECTORIES)
        target_include_directories(${aams_add_target_NAME} PRIVATE ${aams_add_target_INCLUDE_DIRECTORIES})
    endif()

    if (aams_add_target_COMPILE_DEFINITIONS)
        target_compile_definitions(${aams_add_target_NAME} ${aams_add_target_COMPILE_DEFINITIONS})
    endif()

    if (aams_add_target_BUILD_DEPENDENCIES)
        target_link_libraries(${aams_add_target_NAME} ${aams_add_target_BUILD_DEPENDENCIES})
    endif()

    if(aams_add_target_TARGET_PROPERTIES)
        set_target_properties(${aams_add_target_NAME} PROPERTIES ${aams_add_target_TARGET_PROPERTIES})
    endif()

    if (aams_add_target_RUNTIME_LIBRARIES)
        target_link_libraries(${aams_add_target_NAME} ${aams_add_target_RUNTIME_LIBRARIES})
        foreach(runtime_lib ${aams_add_target_RUNTIME_LIBRARIES})
            add_custom_command(TARGET ${aams_add_target_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                    $<TARGET_FILE:${runtime_lib}>
                    $<TARGET_FILE_DIR:${aams_add_target_NAME}>
            )
        endforeach()
        
    endif()


endfunction(aams_add_target)
