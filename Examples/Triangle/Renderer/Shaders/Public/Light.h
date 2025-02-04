#include "Common.h"

struct DirectionalLight
{
    F32_3 color;
    F32   intensity;
    F32_3 direction;
};

struct PointLight
{
    F32_3 color;
    F32   intensity;
    F32_3 position;
};

struct SpotLight
{
    F32_3 color;
    F32   intensity;
    F32_3 position;
    F32   innerAngle;
    F32_3 direction;
    F32   outerAngle;
};

struct AreaLight
{
    F32_3 color;
    F32   intensity;
    F32_3 position;
    F32_3 direction;
    F32_2 size;
};