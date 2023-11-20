cmake_minimum_required(VERSION 3.14)

include(${CMAKE_SOURCE_DIR}/CMake/prelude.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/lint.cmake)

project(
	RHI
	VERSION 0.1.0
	DESCRIPTION "RHI (Render Hardware Interface) is graphics API abstraction layer"
	HOMEPAGE_URL "https://github.com/AbdullahAmrSobh/RHI/"
	LANGUAGES CXX
)

include(${CMAKE_SOURCE_DIR}/CMake/project-is-top-level.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/variables.cmake)

# ---- Declare library ----

add_subdirectory(RHI)
add_subdirectory(Vulkan)

# ---- Install rules ----

# if(NOT CMAKE_SKIP_INSTALL_RULES)
#   include(cmake/install-rules.cmake)
# endif()

# ---- Declare examples ----

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
	message(AUTHOR_WARNING "Developer mode is intended for developers of RHI")
endif()

include(${CMAKE_SOURCE_DIR}/CMake/dev-mode.cmake)