set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Triangle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiManager.cpp
)

tl_add_target(
    NAME Triangle
    NAMESPACE Examples
    EXECUTABLE
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        Examples::Base
)

if (RHI_BACKEND_VULKAN)
    target_link_libraries(Triangle neon::Renderer)
endif()

target_link_libraries(Triangle assimp)

target_compile_definitions(Triangle PRIVATE GLM_FORCE_LEFT_HANDED)
target_compile_definitions(Triangle PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)

target_link_libraries(Triangle rhi_imgui)
