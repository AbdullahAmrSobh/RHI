set( ENKITS_BUILD_EXAMPLES OFF CACHE BOOL "Build basic example applications" )
set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_BUILD_TESTS OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_BUILD_INSTALL OFF CACHE BOOL  "GLFW lib only" )

add_subdirectory(GLFW)

add_executable(Triangle Triangle/Triangle.cpp)
message(STATUS "Adding \"${GLFW_DIR}/include\" as include dir for Examples")
target_include_directories(Triangle PRIVATE ${GLFW_DIR}/include PRIVATE ${INCLUDE_DIR})
target_link_libraries(Triangle RHI)
target_link_libraries(Triangle glfw)

set_property(TARGET Triangle PROPERTY CXX_STANDARD 23)
