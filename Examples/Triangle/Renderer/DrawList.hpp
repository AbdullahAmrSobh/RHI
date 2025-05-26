#pragma once

#include <RHI/RHI.hpp>

#include <TL/Block.hpp>
#include <TL/Memory.hpp>
#include <TL/Span.hpp>
#include <TL/stdint.hpp>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "BufferPool.hpp"
#include "Common.hpp"

#include "Shaders/Public/GPU.h"

namespace Engine
{
    class IndirectDrawList
    {
    public:
        uint32_t m_capacity;
        GpuArray<GPU::DrawRequest> m_drawRequests;
        GpuArray<GPU::MeshUniform> m_uniforms;

        ResultCode Init(RHI::Device* device, uint32_t capacity);
        void       Shutdown(RHI::Device* device);

        void AddStaticMesh(const class StaticMeshLOD* staticMesh, GPU::MeshUniform uniform);
    };
} // namespace Engine