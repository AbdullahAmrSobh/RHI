cmake_minimum_required(VERSION 3.16)

if (MSVC)
#   set(CMAKE_CXX_FLAGS /WX)
endif()

# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Common/Assert.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Common/Flags.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Common/Handle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Common/Span.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Common/Result.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/CommandList.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Format.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/FrameScheduler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Resources.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Swapchain.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/RHI.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/RHI/Attachments.hpp
)

# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Format.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/FrameScheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Pass.cpp
    
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Common/Callstack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Attachments.cpp


    ${CMAKE_SOURCE_DIR}/tracy/public/TracyClient.cpp
)

include(${CMAKE_SOURCE_DIR}/CMake/AddTarget.cmake)
aams_add_target(
    NAME RHI
    NAMESPACE RHI
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
)

target_include_directories(RHI SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/tracy/public")