#pragma once

#include <RHI/RHI.hpp>

#include <TL/Block.hpp>
#include <TL/Span.hpp>
#include <TL/stdint.hpp>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "Renderer/Common.hpp"
#include "Renderer/Resources.hpp"

#include "Shaders/GpuCommonStructs.h"

namespace Engine
{
    struct StaticMeshLOD
    {
    public:
        static TL::Ptr<StaticMeshLOD> create(uint32_t ibSize, uint32_t vbSize);
        static TL::Ptr<StaticMeshLOD> create(
            TL::Span<const uint32_t>  indcies,
            TL::Span<const glm::vec3> positions,
            TL::Span<const glm::vec3> normals,
            TL::Span<const glm::vec2> texcoord);

        StaticMeshLOD(uint32_t indexCount, uint32_t elementsCount);
        ~StaticMeshLOD();

        GPU::StaticMeshIndexed                     m_drawArgs;
        GPUArrayAllocation<GPU::StaticMeshIndexed> m_sbDrawArgs;
        GPUArrayAllocation<uint32_t>               m_indexBuffer;
        GPUArrayAllocation<glm::vec3>              m_vbPositions;
        GPUArrayAllocation<glm::vec3>              m_vbNormals;
        GPUArrayAllocation<glm::vec2>              m_vbTexcoord;
    };
} // namespace Engine