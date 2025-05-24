#pragma once

#include "Shaders//Public//GpuScene.h"

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "BufferPool.hpp"
#include "Common.hpp"

namespace Engine
{
    class SceneView
    {
    public:
        TL::String                           m_name;
        UniformBuffer<GpuScene::SceneView>   m_sceneViewUB;
        StorageBuffer<GpuScene::DrawRequest> m_drawList; // TODO: Replace with TL::Ptr<DrawList>
    };
} // namespace Engine