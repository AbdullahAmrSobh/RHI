#pragma once

#include "Common.h"

namespace GPU
{
    // template<typename T, uint32_t Count>
    // struct Array
    // {
    // };

    // struct CBScene
    // {
    // };

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
        // TODO: Add AABB
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
        U32 meshID;
        U32 materialID;
        U32 instanceID;
        U32 drawViewMask;
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

    struct SceneGlobalConstants
    {
        F32_4 simple_color;
    };

    struct SceneMeshLod
    {
    };

    struct SceneParticleEmitter
    {
    };

    struct SceneLightVisibleRenderablesHash
    {
    };

    struct SceneLight
    {
    };

    struct SceneReflectionProbe
    {
    };

    struct SceneGlobalIlluminationProbe
    {
    };

    struct SceneDecal
    {
    };

    struct SceneFogDensityVolume
    {
    };

    struct SceneRenderable
    {
        // U32 m_worldTransformsIndex; ///< First index points to the crnt transform and the 2nd to the previous.
        // U32 m_constantsOffset;
        // U32 m_meshLodsIndex;                    ///< Points to the array of GpuSceneMeshLod. kMaxLodCount are reserved for each renderable.
        // U32 m_boneTransformsOffset;             ///< Array of Mat3x4 or 0 if its not a skin.
        // U32 m_particleEmitterIndex;             ///< Index to the GpuSceneParticleEmitter array or kMaxU32 if it's not an emitter.
        // U32 m_rtShadowsShaderHandleIndex;       ///< The index of the shader handle in the array of library's handles.
        // U32 m_rtMaterialFetchShaderHandleIndex; ///< The index of the shader handle in the array of library's handles.
        // U32 m_uuid;

        U32 instanceID;
        U32 materialID;
        U32 meshID;
    };

    struct SceneRenderableBoundingVolume
    {
    };

} // namespace GPU
