#include "Mesh.hpp"

namespace Engine
{
    MeshUnifiedGeometryBuffer::MeshUnifiedGeometryBuffer()
        : m_allocator{Suballocator(0), Suballocator(0), Suballocator(0), Suballocator(0)}
    {
    }

    ResultCode MeshUnifiedGeometryBuffer::Init(RHI::Device& device, size_t vertexCount)
    {
        m_device      = &device;
        m_vertexCount = vertexCount;

        RHI::BufferCreateInfo unifiedGeometryBufferCI{
            .name       = "Unified Geometry Buffer",
            .usageFlags = RHI::BufferUsage::Index | RHI::BufferUsage::Vertex,
            .byteSize   = (m_vertexCount) * (sizeof(uint32_t) + (2 * sizeof(glm::vec3)) + sizeof(glm::vec2)),
        };
        auto [buffer, result] = m_device->CreateBuffer(unifiedGeometryBufferCI);
        m_buffer              = buffer;

        /// @note offsets initialization must happen in this order!

        // Initialize offsets
        m_vertexAttributeOffsets[U32(MeshAttributeType::Index)]    = 0;
        m_vertexAttributeOffsets[U32(MeshAttributeType::Position)] = m_vertexAttributeOffsets[U32(MeshAttributeType::Index)] + vertexCount * sizeof(uint32_t);
        m_vertexAttributeOffsets[U32(MeshAttributeType::Normal)]   = m_vertexAttributeOffsets[U32(MeshAttributeType::Position)] + vertexCount * sizeof(glm::vec3);
        m_vertexAttributeOffsets[U32(MeshAttributeType::Uv)]       = m_vertexAttributeOffsets[U32(MeshAttributeType::Normal)] + vertexCount * sizeof(glm::vec3);

        // Initialize suballocators with correct sizes
        new (&m_allocator[U32(MeshAttributeType::Index)]) Suballocator(vertexCount * sizeof(uint32_t));
        new (&m_allocator[U32(MeshAttributeType::Position)]) Suballocator(vertexCount * sizeof(glm::vec3));
        new (&m_allocator[U32(MeshAttributeType::Normal)]) Suballocator(vertexCount * sizeof(glm::vec3));
        new (&m_allocator[U32(MeshAttributeType::Uv)]) Suballocator(vertexCount * sizeof(glm::vec2));
        return result;
    }

    void MeshUnifiedGeometryBuffer::Shutdown()
    {
        m_device->DestroyBuffer(m_buffer);
    }

    RHI::BufferBindingInfo MeshUnifiedGeometryBuffer::GetIndexBuffer() const
    {
        return GetVertexBuffer(MeshAttributeType::Index);
    }

    RHI::BufferBindingInfo MeshUnifiedGeometryBuffer::GetVertexBuffer(MeshAttributeType attribute) const
    {
        return RHI::BufferBindingInfo{
            .buffer = m_buffer,
            .offset = m_vertexAttributeOffsets[U32(attribute)],
        };
    }

    StaticMesh MeshUnifiedGeometryBuffer::CreateMesh(
        const char*               name,
        TL::Span<const uint32_t>  indcies,
        TL::Span<const glm::vec3> positions,
        TL::Span<const glm::vec3> normals,
        TL::Span<const glm::vec2> uvs)
    {
        // Validate inputs
        if (indcies.empty() || positions.empty() || normals.empty() || uvs.empty())
        {
            TL_ASSERT(false, "Invalid input spans");
            return StaticMesh{};
        }

        if (indcies.size() > m_vertexCount || positions.size() > m_vertexCount)
        {
            TL_ASSERT(false, "Input size exceeds buffer capacity");
            return StaticMesh{};
        }

        StaticMesh staticMesh{};

        staticMesh.indciesSuballocation   = m_allocator[U32(MeshAttributeType::Index)].allocate(indcies.size_bytes());
        staticMesh.positionsSuballocation = m_allocator[U32(MeshAttributeType::Position)].allocate(positions.size_bytes());
        staticMesh.normalsSuballocation   = m_allocator[U32(MeshAttributeType::Normal)].allocate(normals.size_bytes());
        staticMesh.uvsSuballocation       = m_allocator[U32(MeshAttributeType::Uv)].allocate(uvs.size_bytes());

        staticMesh.indcies.buffer   = m_buffer;
        staticMesh.positions.buffer = m_buffer;
        staticMesh.normals.buffer   = m_buffer;
        staticMesh.uvs.buffer       = m_buffer;
        staticMesh.indcies.offset   = m_vertexAttributeOffsets[U32(MeshAttributeType::Index)]    + staticMesh.indciesSuballocation.offset;
        staticMesh.positions.offset = m_vertexAttributeOffsets[U32(MeshAttributeType::Position)] + staticMesh.positionsSuballocation.offset;
        staticMesh.normals.offset   = m_vertexAttributeOffsets[U32(MeshAttributeType::Normal)]   + staticMesh.normalsSuballocation.offset;
        staticMesh.uvs.offset       = m_vertexAttributeOffsets[U32(MeshAttributeType::Uv)]       + staticMesh.uvsSuballocation.offset;

        auto ptr = m_device->MapBuffer(m_buffer);
        memcpy((char*)ptr + staticMesh.indcies.offset,    indcies.data(),   indcies.size_bytes());
        memcpy((char*)ptr + staticMesh.positions.offset, positions.data(), positions.size_bytes());
        memcpy((char*)ptr + staticMesh.normals.offset,   normals.data(),   normals.size_bytes());
        memcpy((char*)ptr + staticMesh.uvs.offset,       uvs.data(),       uvs.size_bytes());
        m_device->UnmapBuffer(m_buffer);

        staticMesh.parameters.elementsCount = indcies.size();
        staticMesh.parameters.firstElement = 0;
        staticMesh.parameters.vertexOffset  = 0;

        return staticMesh;
    }

} // namespace Engine