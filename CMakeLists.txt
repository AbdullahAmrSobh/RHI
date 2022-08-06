cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(NAME RHI)

project(${NAME})

# Use FindVulkan module added with CMAKE 3.7
if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
	message(STATUS "Using module to find Vulkan")
	find_package(Vulkan)
endif()

if (NOT VULKAN_FOUND) 
    message(FATEL_ERROR "Failed to find Vulkan")
endif()

include(FILES.cmake)

add_library(RHI_INTERFACE INTERFACE ${HEADER_FILES})

set(SOURCE_FILES_DIR = ${CMAKE_SOURCE_DIR}/Source)
set(INCLUDE_DIR = ${CMAKE_SOURCE_DIR}/Include)

# BUILD RHI as a static library
add_library(RHI STATIC ${SOURCE_FILES})
target_include_directories(RHI PUBLIC ${INCLUDE_DIR}
							   PRIVATE ${SOURCE_FILES_DIR}
							   PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(RHI PRIVATE ${Vulkan_LIBRARIES})

set_property(TARGET RHI PROPERTY CXX_STANDARD 23)

add_subdirectory(Examples)