cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)
include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(cmake/CPM.cmake)

project(
    RHI
    VERSION 0.1.0
    DESCRIPTION "Renderer Hardware Interface"
    HOMEPAGE_URL "https://github.com/AbdullahAmrSobh/RHI"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Declare library ----

add_subdirectory(RHI)
add_subdirectory(Vulkan)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

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