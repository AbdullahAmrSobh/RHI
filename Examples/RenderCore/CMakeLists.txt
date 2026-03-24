set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
)

tl_add_target(
    NAME RenderCore
    NAMESPACE Examples
    STATIC
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        glm
        TL
        RHI::RHI
)
