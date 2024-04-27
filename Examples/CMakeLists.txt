include(${CMAKE_SOURCE_DIR}/CMake/CPM.cmake)

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