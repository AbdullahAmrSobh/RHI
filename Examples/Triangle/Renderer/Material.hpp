#pragma once

#include <RHI/RHI.hpp>

#include <glm/glm.hpp>

#include "Bindless.hpp"

namespace Engine
{
    class Material
    {
    };

    class BSDFMaterial
    {
    public:
        BindlessTextureHandle m_albedoMap;
        glm::vec4             m_albedoColor;

        BindlessTextureHandle m_normalMap;

        BindlessTextureHandle m_metallicRoughnessMap;
        float                 m_metallic           = 0.0f;
        float                 m_roughness          = 0.5f;
        float                 m_reflectance        = 0.04f;
        float                 m_clearCoat          = 0.0;
        float                 m_clearCoatRoughness = 0.0;
    };
} // namespace Engine