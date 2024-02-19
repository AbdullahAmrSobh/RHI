include(${CMAKE_SOURCE_DIR}/CMake/CPM.cmake)

# Add dependencies
CPMAddPackage(
    NAME           RHI_spdlog
	GIT_REPOSITORY git@github.com:gabime/spdlog.git
	GIT_TAG        v1.12.0
)

CPMAddPackage(
    NAME           RHI_GLFW
	GIT_REPOSITORY git@github.com:glfw/glfw.git
	GIT_TAG        3.3.8
)

CPMAddPackage(
	NAME           RHI_glm
	GIT_REPOSITORY git@github.com:g-truc/glm.git
	GIT_TAG        0.9.9.8
	OPTIONS
		"GLM_CONFIG_CLIP_CONTROL ON"
		"GLM_CLIP_CONTROL_ZO_BIT ON"
)

add_subdirectory(ImGui-1.90.0)

# Add examples
add_subdirectory(Base)
add_subdirectory(Triangle)