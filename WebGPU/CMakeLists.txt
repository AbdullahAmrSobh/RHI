set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.hpp
)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/CommandList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
)

tl_add_target(
    NAME WebGPU
    NAMESPACE RHI
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RHI::RHI
)

if (NOT EMSCRIPTEN)
    target_link_libraries(WebGPU
        webgpu_dawn
        dawn_common
        dawn_native
        dawn_headers
    )
else()
    target_link_libraries(WebGPU webgpu_headers)
endif()
