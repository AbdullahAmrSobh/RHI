#pragma once

#include <RHI/RHI.hpp>

#include <TL/Block.hpp>
#include <TL/Memory.hpp>
#include <TL/Span.hpp>
#include <TL/stdint.hpp>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "Common.hpp"

namespace Engine
{
    enum class MeshAttributeType : U32
    {
        Index = 0,
        Position,
        Normal,
        Uv,
        Count
    };

    struct AABB
    {
        glm::vec3 min;
        glm::vec3 max;
    };

    struct MeshAttribute
    {
        //
    };

    class MeshResource
    {
    public:
        struct LOD
        {
            MeshAttribute m_indcies;
            MeshAttribute m_attributes[uint32_t(MeshAttributeType::Count)];
            uint32_t      m_indexCount;
            uint32_t      m_vertexCount;
            uint32_t      m_meshletCount;
        };

        AABB GetAABB() const { return m_aabb; }

        const TL::Span<const LOD> GetLODs() const { return m_lod; }

        const LOD& GetLOD(size_t index) const { return m_lod[index]; }

    private:
        AABB            m_aabb;
        TL::Vector<LOD> m_lod;
    };

    struct StaticMesh
    {
        RHI::BufferBindingInfo indcies;
        Suballocation          indciesSuballocation;

        RHI::BufferBindingInfo positions;
        Suballocation          positionsSuballocation;

        RHI::BufferBindingInfo normals;
        Suballocation          normalsSuballocation;

        RHI::BufferBindingInfo uvs;
        Suballocation          uvsSuballocation;

        RHI::DrawIndexedParameters parameters;
    };

    class MeshUnifiedGeometryBuffer
    {
    public:
        MeshUnifiedGeometryBuffer();

        /// @brief Initializes the mesh factory
        ResultCode Init(RHI::Device& device, size_t vertexCount);

        /// @brief Cleans up and releases resources
        void Shutdown();

        RHI::BufferBindingInfo GetIndexBuffer() const;
        RHI::BufferBindingInfo GetVertexBuffer(MeshAttributeType attribute) const;

        StaticMesh CreateMesh(
            const char*               name,
            TL::Span<const uint32_t>  indcies,
            TL::Span<const glm::vec3> positions,
            TL::Span<const glm::vec3> normals,
            TL::Span<const glm::vec2> uvs);

    private:
        RHI::Device*             m_device;
        RHI::Handle<RHI::Buffer> m_buffer;
        Suballocator             m_allocator[U32(MeshAttributeType::Count)];
        size_t                   m_vertexCount;
        size_t                   m_vertexAttributeOffsets[U32(MeshAttributeType::Count)];
    };
} // namespace Engine