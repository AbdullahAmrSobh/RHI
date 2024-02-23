cmake_minimum_required(VERSION 3.14)

include(cmake/variables.cmake)
include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(RHI_DEVELOPER_MODE ON)

option(TRACY_ENABLE ON)
add_compile_definitions(TRACY_ENABLE)
option(RHI_DEBUG ON)
add_compile_definitions(RHI_DEBUG)

# ---- In-source guard ----

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
      FATAL_ERROR
      "In-source builds are not supported. "
      "Please read the BUILDING document before trying to build this project. "
      "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first."
  )
endif()

project(
    RHI
    VERSION 0.1.0
    DESCRIPTION "Renderer Hardware Interface"
    HOMEPAGE_URL "https://github.com/AbdullahAmrSobh/RHI"
    LANGUAGES CXX
)

# This variable is set by project() in CMake 3.21+
string(
    COMPARE EQUAL
    "${CMAKE_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}"
    PROJECT_IS_TOP_LEVEL
)

# ---- Declare library ----

add_subdirectory(RHI)
add_subdirectory(Vulkan)

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${RHI_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(Examples)
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