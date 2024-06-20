cmake_minimum_required(VERSION 3.14)

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/imgui.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/imgui_tables.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/imgui_demo.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/imgui_draw.cpp
)

set(HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/imstb_truetype.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/imstb_textedit.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/imstb_rectpack.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/imgui.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/imgui_internal.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h
)

include(${CMAKE_SOURCE_DIR}/CMake/add-target.cmake)
aams_add_target(
    NAME ImGui
    NAMESPACE Thirdparty
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
    BUILD_DEPENDENCIES
        RHI::RHI
    RUNTIME_LIBRARIES
        RHI::Vulkan
)

target_include_directories(ImGui ${warning_guard} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set compiler flags for your target
target_compile_options(ImGui PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/W3>" "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall;-Wextra>")
