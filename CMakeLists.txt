cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    RHI
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(cmake/source-files.cmake)

# ---- Packages ----

# Find the Vulkan package
find_package(Vulkan REQUIRED)

# ---- Declare library ----

add_library(RHI_RHI ${SOURCE_FILES})
add_library(RHI::RHI ALIAS RHI_RHI)

include(GenerateExportHeader)
generate_export_header(
    RHI_RHI
    BASE_NAME RHI
    EXPORT_FILE_NAME export/RHI/RHI_export.hpp
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(RHI_RHI PUBLIC RHI_STATIC_DEFINE)
endif()

set_target_properties(
    RHI_RHI PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME RHI
    OUTPUT_NAME RHI
)

target_include_directories(
    RHI_RHI ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    RHI_RHI SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(RHI_RHI PUBLIC cxx_std_20)

target_link_libraries(RHI_RHI Vulkan::Vulkan)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${RHI_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT RHI_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of RHI"
  )
endif()

include(cmake/dev-mode.cmake)
