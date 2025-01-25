#pragma once

#include "Common.h"
#include "Light.h"

/// Contains transformation matrices and lighting information for the GPU scene.
struct Scene
{
    F32_4x4 worldToViewMatrix;      /// World to view transformation matrix.
    F32_4x4 viewToClipMatrix;       /// View to clip transformation matrix.
    F32_4x4 worldToClipMatrix;      /// World to clip transformation matrix.
    F32_4x4 clipToViewMatrix;       /// Clip to view transformation matrix.
    F32_4x4 viewToWorldMatrix;      /// View to world transformation matrix.
    F32_4x4 clipToWorldMatrix;      /// Clip to world transformation matrix.
    F32_3   cameraPosition;         /// Camera position in world space.
    U32     directionalLightsCount; /// Number of directional lights.
    U32     pointLightsCount;       /// Number of point lights.
    U32     spotLightsCount;        /// Number of spot lights.
    U32     areaLightsCount;        /// Number of area lights.
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
    F32_4x4     modelToWorldMatrix;
    PbrMaterial material;
};
