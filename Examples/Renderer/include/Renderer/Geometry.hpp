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
        {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerVertex,   {{0, RHI::Format::RGB32_FLOAT}} }, // position
        {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerVertex,   {{0, RHI::Format::RGB32_FLOAT}} }, // normal
        {sizeof(glm::vec2), RHI::PipelineVertexInputRate::PerVertex,   {{0, RHI::Format::RG32_FLOAT}}  }, // texcoord
        {sizeof(glm::vec3), RHI::PipelineVertexInputRate::PerInstance, {{0, RHI::Format::RGBA32_FLOAT}}}, // draw-id
    };

    enum class MeshAttributeType
    {
        Index,
        Position,
        Normal,
        TexCoord,
        Count,
    };

    template<typename T>
    class MeshAttribute
    {
        U32       m_elementCount;
        Buffer<T> m_buffer;
    };

    struct StaticMeshLOD
    {
    public:
        GPU::StaticMeshIndexed m_drawArgs;
        uint32_t               m_indirectDrawArgs;

        MeshAttribute<uint32_t>  m_indexAttribute;
        MeshAttribute<glm::vec3> m_positionAttribute;
        MeshAttribute<glm::vec3> m_normalAttribute;
        MeshAttribute<glm::vec2> m_uvAttribute;
    };

    StaticMeshLOD CreateStaticMeshLOD(
        TL::Span<const uint32_t>  indicies,
        TL::Span<const glm::vec3> positions,
        TL::Span<const glm::vec3> normals,
        TL::Span<const glm::vec2> uvs);

    StaticMeshLOD CreateStaticMeshLOD(U32 vertexCount, U32 indexCount);

    void ReleaseStaticMeshLOD(StaticMeshLOD lod);


} // namespace Engine