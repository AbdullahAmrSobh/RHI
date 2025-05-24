#pragma once

#include "Common.h"
#include "Light.h"

namespace GpuScene
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
        F32_4x4 worldToClipMatrix;
        F32_4x4 clipToWorldMatrix;
    };
} // namespace GpuScene
