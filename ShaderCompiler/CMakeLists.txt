set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen.cpp
)

tl_add_target(
    NAME ShaderCompiler
    NAMESPACE Engine
    EXECUTABLE
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RHI::RHI
        RHI::Vulkan
        slang
)
