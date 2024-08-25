cmake_minimum_required(VERSION 3.14)

set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets/Buffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets/Definitions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets/Export.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets/Image.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets/Importer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets/Material.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets/Mesh.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets/Name.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets/Package.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets/SceneGraph.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Assets/SerializeGLM.hpp
)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/external
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/AssetFile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Importer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Material.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Mesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Package.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/SceneGraph.cpp
)

include(${CPM_TL_SOURCE}/CMake/add-target.cmake)
tl_add_target(
    NAME Assets
    STATIC
    HEADERS ${HEADER_FILES}
    SOURCES ${SOURCE_FILES}
)

target_link_libraries(Assets PUBLIC TL)
target_link_libraries(Assets PUBLIC glm)
target_link_libraries(Assets PRIVATE assimp)
target_link_libraries(Assets PRIVATE CMP_Core)
target_link_libraries(Assets PRIVATE CMP_Core_AVX)
target_link_libraries(Assets PRIVATE CMP_Core_SSE)
# target_link_libraries(Assets PRIVATE CMP_GpuDecode)
target_link_libraries(Assets PRIVATE CMP_Compressonator)
target_link_libraries(Assets PRIVATE CMP_Framework)
# target_link_libraries(Assets PRIVATE meshoptimizer)

target_include_directories(Assets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)