cmake_minimum_required(VERSION 3.14)

project(RHIExamples CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(FetchContent)

if(PROJECT_IS_TOP_LEVEL)
  find_package(RHI REQUIRED)
endif()

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
)

FetchContent_GetProperties(glfw)

if(NOT glfw_POPULATED)
  FetchContent_Populate(glfw)
  set(GLFW_STANDALONE OFF CACHE INTERNAL "Build cmake as standalone")
  set(glfw_PKG_DEPS OFF CACHE INTERNAL "Build cmake with waylane")
  set(GLFW_USE_WAYLAND OFF CACHE INTERNAL "Build cmake with waylane")
  set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
  set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
  set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
  set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

  add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()

add_custom_target(run-examples)

function(add_example NAME)
  add_executable("${NAME}" "${NAME}.cpp")
  target_link_libraries("${NAME}" PRIVATE RHI::RHI glfw)
  target_compile_features("${NAME}" PRIVATE cxx_std_20)
  add_custom_target("run_${NAME}" COMMAND "${NAME}" VERBATIM)
  add_dependencies("run_${NAME}" "${NAME}")
  add_dependencies(run-examples "run_${NAME}")
endfunction()

add_example(triangle)
add_folders(Example)
