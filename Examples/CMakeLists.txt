include(${CMAKE_SOURCE_DIR}/CMake/CPM.cmake)

# Add dependencies
CPMAddPackage(
    NAME           RHI_spdlog
	GIT_REPOSITORY git@github.com:gabime/spdlog.git
	GIT_TAG        1.12.0
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
)

CPMAddPackage(
	NAME           RHI_assimp
	GIT_REPOSITORY git@github.com:assimp/assimp.git
	GIT_TAG        v5.3.1
	OPTIONS
		# "BUILD_SHARED_LIBS ON"
		# "ASSIMP_BUILD_FRAMEWORK OFF"
		# "ASSIMP_DOUBLE_PRECISION OFF"
		# "ASSIMP_OPT_BUILD_PACKAGES OFF"
		# "ASSIMP_ANDROID_JNIIOSYSTEM OFF"
		"ASSIMP_NO_EXPORT OFF"
		# "ASSIMP_BUILD_ASSIMP_TOOLS OFF"
		# "ASSIMP_BUILD_SAMPLES OFF"
		"ASSIMP_BUILD_TESTS OFF"
		# "ASSIMP_COVERALLS OFF"
		# "ASSIMP_INSTALL ON"
		# "ASSIMP_WARNINGS_AS_ERRORS OFF"
		# "ASSIMP_ASAN OFF"
		# "ASSIMP_UBSAN OFF"
		# "ASSIMP_BUILD_DOCS OFF"
		# "ASSIMP_INJECT_DEBUG_POSTFIX ON"
		# "ASSIMP_IGNORE_GIT_HASH OFF"
		# "ASSIMP_BUILD_ZLIB ON"
)

# Add examples
add_subdirectory(Base)
add_subdirectory(Triangle)