#include <RHI/RHI.hpp>

#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>
#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Log.hpp>
#include <TL/Utils.hpp>

#include <RHI-Vulkan/Loader.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <tracy/Tracy.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include "Camera.hpp"
#include "dds_image/dds.hpp"
#include "stb_image.h"

using namespace Examples;

template<typename T>
using Handle = RHI::Handle<T>;

namespace Shader // TODO: this should be reflected from slang-shaders using some tool
{
#include "Shaders/Public/GPU.h"

    inline constexpr auto kUniformMinOffsetAlignment = 64;
    inline constexpr auto kAlignedSceneSize          = (sizeof(Scene) + kUniformMinOffsetAlignment - 1) & ~(kUniformMinOffsetAlignment - 1);
    inline constexpr auto kAlignedPerDrawSize        = (sizeof(Drawable) + kUniformMinOffsetAlignment - 1) & ~(kUniformMinOffsetAlignment - 1);
} // namespace Shader

#pragma region fastgltf_type_conversions

template<>
struct fastgltf::ComponentTypeConverter<glm::vec2>
{
    static constexpr auto type = ComponentType::Float;
};

template<>
struct fastgltf::ComponentTypeConverter<glm::vec3>
{
    static constexpr auto type = ComponentType::Float;
};

template<>
struct fastgltf::ComponentTypeConverter<glm::vec4>
{
    static constexpr auto type = ComponentType::Float;
};

// TODO: add more as needed

inline static glm::mat4 convertMatrix(const fastgltf::math::fmat4x4& matrix)
{
    return {
        {matrix.col(0).x(), matrix.col(0).y(), matrix.col(0).z(), matrix.col(0).w()},
        {matrix.col(1).x(), matrix.col(1).y(), matrix.col(1).z(), matrix.col(1).w()},
        {matrix.col(2).x(), matrix.col(2).y(), matrix.col(2).z(), matrix.col(2).w()},
        {matrix.col(3).x(), matrix.col(3).y(), matrix.col(3).z(), matrix.col(3).w()},
    };
}

template<typename T>
inline static glm::vec2 convertVec3(const fastgltf::math::vec<T, 2>& vec)
{
    return {vec.x(), vec.y()};
}

template<typename T>
inline static glm::vec3 convertVec3(const fastgltf::math::vec<T, 3>& vec)
{
    return {vec.x(), vec.y(), vec.z()};
}

template<typename T>
inline static glm::vec4 convertVec4(const fastgltf::math::vec<T, 4>& vec)
{
    return {vec.x(), vec.y(), vec.z(), vec.w()};
}

inline static glm::quat convertQuat(const fastgltf::math::fquat& quat)
{
    return {quat.x(), quat.y(), quat.z(), quat.w()};
}

#pragma endregion

