#pragma once

#include "Common.h"
#include "Light.h"

#ifdef __cplusplus
namespace Shader
{
#endif

    struct ObjectID
    {
        U32 objectID;
        U32 materialID;
        U32 transformID;
    };

    /// Contains transformation matrices and lighting information for the GPU scene.
    struct SceneView
    {
        F32_4x4 worldToViewMatrix; /// World to view transformation matrix.
        F32_4x4 viewToClipMatrix;  /// View to clip transformation matrix.
        F32_4x4 worldToClipMatrix; /// World to clip transformation matrix.
        F32_4x4 clipToViewMatrix;  /// Clip to view transformation matrix.
        F32_4x4 viewToWorldMatrix; /// View to world transformation matrix.
        F32_4x4 clipToWorldMatrix; /// Clip to world transformation matrix.
        F32_3   cameraPosition;    /// Camera position in world space.
    };

    struct PbrMaterial
    {
        F32_4 albedo;
        U32   albedoMapIndex;
        U32   normalMapIndex;
        F32   roughness;
        F32   metallic;
        U32   metallicRoughnessMapIndex;
    };

    struct Drawable
    {
        F32_3x4 modelToWorldMatrix;
    };

    /// @brief Parameters for drawing primitives.
    struct DrawParameters
    {
        uint32_t vertexCount;   ///< Number of vertices to draw.
        uint32_t instanceCount; ///< Number of instances to draw.
        uint32_t firstVertex;   ///< Index of the first vertex to draw.
        uint32_t firstInstance; ///< Index of the first instance to draw.
    };

    /// @brief Parameters for drawing indexed primitives.
    struct DrawIndexedParameters
    {
        uint32_t indexCount;    ///< Number of indices to draw.
        uint32_t instanceCount; ///< Number of instances to draw.
        uint32_t firstIndex;    ///< Index of the first index to draw.
        int32_t  vertexOffset;  ///< Offset added to each index.
        uint32_t firstInstance; ///< Index of the first instance to draw.
    };

#ifdef __cplusplus
} // namespace Shader
#endif