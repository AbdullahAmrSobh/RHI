# Add GLFW for window
include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw)

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

# Add examples
add_subdirectory(Base)
add_subdirectory(Triangle)