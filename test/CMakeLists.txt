cmake_minimum_required(VERSION 3.14)

project(RHITests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(RHI REQUIRED)
  enable_testing()
endif()

# ---- Tests ----

add_executable(RHI_test source/RHI_test.cpp)
target_link_libraries(RHI_test PRIVATE RHI::RHI)
target_compile_features(RHI_test PRIVATE cxx_std_20)

add_test(NAME RHI_test COMMAND RHI_test)
windows_set_path(RHI_test RHI::RHI)

# ---- End-of-file commands ----

add_folders(Test)
