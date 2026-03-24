set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/ApplicationBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Window.cpp
)

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
