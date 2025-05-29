#pragma once

#include "Common.h"

namespace GPU
{
    struct DrawParameters // Matches RHI
    {
        U32 vertexCount;
        U32 instanceCount;
        U32 firstVertex;
        U32 firstInstance;
    };

    struct DrawIndexedParameters // Matches RHI
    {
        U32 indexCount;
        U32 instanceCount;
        U32 firstIndex;
        I32 vertexOffset;
        U32 firstInstance;
    };

    struct StaticMeshNonIndexed
    {
        U32 vertexCount;
        U32 firstVertex;
    };

    struct StaticMeshIndexed
    {
        U32 indexCount;
        U32 firstIndex;
        I32 vertexOffset;
    };

    struct MeshMaterialBindless
    {
        F32_4 albedo;
        F32_2 roughnessMetallic;
        U32   albedoTextureId;
        U32   normalTextureId;
        U32   roughnessMetallicTextureId;
    };

    struct MeshUniform
    {
        F32_4x4 modelToWorldMatrix;
        U32     materialId;
        U32     _padding[3];
    };

    struct DrawRequest
    {
        U32 meshId;
        U32 materialId;
        U32 uniformId;
    };

    struct SceneView
    {
        F32_4x4 worldToViewMatrix;
        F32_4x4 viewToClipMatrix;
    };

    /// Stub

    struct DirectionalLight
    {
        F32_3 color;
        F32   intensity;
        F32_3 direction;

        static constexpr uint32_t Weight = 0;
    };

    struct SpotLight
    {
        F32_3 color;
        F32   intensity;
        F32_3 position;

        static constexpr uint32_t Weight = 2;
    };

    struct AreaLight
    {
        F32_3 color;
        F32   intensity;
        F32_3 position;
        F32   innerAngle;
        F32_3 direction;
        F32   outerAngle;

        static constexpr uint32_t Weight = 3;
    };

    struct PointLight
    {
        F32_3 color;
        F32   intensity;
        F32_3 position;
        F32_3 direction;
        F32_2 size;

        static constexpr uint32_t Weight = 4;
    };
} // namespace GPU
