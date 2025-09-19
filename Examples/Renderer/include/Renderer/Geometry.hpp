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
    static RHI::PipelineVertexBindingDesc UGB_VertexLayout[] = {
        {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerVertex, {{0, RHI::Format::RGB32_FLOAT}}},    // position
        {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerVertex, {{0, RHI::Format::RGB32_FLOAT}}},    // normal
        {sizeof(glm::vec2), RHI::PipelineVertexInputRate::PerVertex, {{0, RHI::Format::RG32_FLOAT}}},     // texcoord
        {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerInstance, {{0, RHI::Format::RGBA32_FLOAT}}}, // draw-id
    };

    struct StaticMeshLOD
    {
    public:
        static TL::Ptr<StaticMeshLOD> create(uint32_t ibSize, uint32_t vbSize);
        static TL::Ptr<StaticMeshLOD> create(
            TL::Span<const uint32_t>  indcies,
            TL::Span<const glm::vec3> positions,
            TL::Span<const glm::vec3> normals,
            TL::Span<const glm::vec2> texcoord);

        ~StaticMeshLOD();

    private:
        StaticMeshLOD(uint32_t indexCount, uint32_t elementsCount);

        GPU::StaticMeshIndexed                   m_drawArgs;
        StructuredBuffer<GPU::StaticMeshIndexed> m_sbDrawArgs;
        MeshBuffer<uint32_t>                     m_indexBuffer;
        MeshBuffer<glm::vec3>                    m_vbPositions;
        MeshBuffer<glm::vec3>                    m_vbNormals;
        MeshBuffer<glm::vec2>                    m_vbTexcoord;
    };
} // namespace Engine