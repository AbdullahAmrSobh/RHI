#include "Common.h"

#ifdef __cplusplus
namespace Shader
{
#endif
    enum class Type
    {
        DirectionalLight,
        PointLight,
        SpotLight,
        AreaLight,
    };

    struct DirectionalLight
    {
        F32_3 color;
        F32   intensity;
        F32_3 direction;

        static constexpr uint32_t Weight = 0;
    };

    struct PointLight
    {
        F32_3 color;
        F32   intensity;
        F32_3 position;

        static constexpr uint32_t Weight = 2;
    };

    struct SpotLight
    {
        F32_3 color;
        F32   intensity;
        F32_3 position;
        F32   innerAngle;
        F32_3 direction;
        F32   outerAngle;

        static constexpr uint32_t Weight = 3;
    };

    struct AreaLight
    {
        F32_3 color;
        F32   intensity;
        F32_3 position;
        F32_3 direction;
        F32_2 size;

        static constexpr uint32_t Weight = 4;
    };

    struct Light
    {

    };

#ifdef __cplusplus
} // namespace Shader
#endif
