cmake_minimum_required(VERSION 3.19)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ApplicationBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Window.cpp
)

if(NOT TARGET TL)
    include(${CMAKE_SOURCE_DIR}/CMake/add-target.cmake)
else()
    include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
endif()

tl_add_target(
    NAME Base
    NAMESPACE Examples
    STATIC
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        glm
        TL
)

if (NOT EMSCRIPTEN)
        target_link_libraries(Base glfw)
endif()