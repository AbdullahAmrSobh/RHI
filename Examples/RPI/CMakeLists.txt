set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/AssimpSceneLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/DeferredRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer.cpp
)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/AssimpSceneLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/DeferredRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer.cpp
)

include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME RPI
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RHI::RHI
        TL
        assimp # should remove
        Examples::Base # needed for window
    RUNTIME_LIBRARIES
        RHI::Vulkan
)