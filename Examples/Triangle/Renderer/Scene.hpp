#pragma once

#include "Shaders//Public//GPU.h"

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
        TL::String                         m_name;
        UniformBuffer<GPU::SceneView> m_uniformBuffer;
        class IndirectDrawList*            m_drawList;
    };
} // namespace Engine