#include "Mesh.hpp"

namespace Engine
{
    static constexpr RHI::Format kMeshAttributeFormat[U32(MeshAttributeType::Count)] = {
        RHI::Format::R32_UINT,    // Index
        RHI::Format::RGB32_FLOAT, // Position
        RHI::Format::RGB32_FLOAT, // Normal
        RHI::Format::RG32_FLOAT,  // TexCoord
    };
    constexpr static U32 kVertexCount = 6; // 64k vertices

    UnifiedGeometryBufferPool::UnifiedGeometryBufferPool() = default;

    ResultCode UnifiedGeometryBufferPool::Init(RHI::Device& device)
    {
        auto sizeIndex    = kVertexCount * sizeof(uint32_t);
        auto sizePosition = kVertexCount * sizeof(glm::vec3);
        auto sizeNormal   = kVertexCount * sizeof(glm::vec3);
        auto sizeTexCoord = kVertexCount * sizeof(glm::vec2);

        m_allocators[U32(MeshAttributeType::Index)].init(sizeIndex);
        m_allocators[U32(MeshAttributeType::Position)].init(sizePosition);
        m_allocators[U32(MeshAttributeType::Normal)].init(sizeNormal);
        m_allocators[U32(MeshAttributeType::TexCoord)].init(sizeTexCoord);

        // initialize segement offsets
        m_segmentStartingOffsets[U32(MeshAttributeType::Index)]    = 0;
        m_segmentStartingOffsets[U32(MeshAttributeType::Position)] = sizeIndex;
        m_segmentStartingOffsets[U32(MeshAttributeType::Normal)]   = sizeIndex + sizePosition;
        m_segmentStartingOffsets[U32(MeshAttributeType::TexCoord)] = sizeIndex + sizePosition + sizeNormal;

        m_device                         = &device;
        size_t                vertexSize = (sizeof(uint32_t) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2));
        RHI::BufferCreateInfo createInfo{
            .name       = "UnifiedGeometryBuffer",
            .hostMapped = true,
            .usageFlags = RHI::BufferUsage::Vertex | RHI::BufferUsage::Index,
            .byteSize   = kVertexCount * vertexSize,
        };
        auto [buffer, result] = device.CreateBuffer(createInfo);
        m_buffer              = buffer;
        return result;
    }

    void UnifiedGeometryBufferPool::Shutdown()
    {
        m_device->DestroyBuffer(m_buffer);
    }

    RHI::BufferBindingInfo UnifiedGeometryBufferPool::GetAttributeBindingInfo(MeshAttributeType attribute) const
    {
        auto offset = m_segmentStartingOffsets[U32(attribute)];
        return {m_buffer, offset};
    }

    void UnifiedGeometryBufferPool::BeginUpdate()
    {
        m_mappedPtr = m_device->MapBuffer(m_buffer);
    }

    void UnifiedGeometryBufferPool::EndUpdate()
    {
        m_device->UnmapBuffer(m_buffer);
        m_mappedPtr = nullptr;
    }

    StaticMeshLOD* UnifiedGeometryBufferPool::CreateStaticMeshLOD(U32 vertexCount, U32 indexCount)
    {
        auto staticMesh           = TL::Allocator::Construct<StaticMeshLOD>();
        staticMesh->m_indexCount  = indexCount;
        staticMesh->m_vertexCount = vertexCount;
        staticMesh->m_indexOffset = m_segmentStartingOffsets[U32(MeshAttributeType::Index)] / sizeof(uint32_t);
        staticMesh->m_vertexCount = m_segmentStartingOffsets[U32(MeshAttributeType::Position)] / sizeof(glm::vec3);

        staticMesh->m_indexAttribute    = CreateMeshAttribute(indexCount, MeshAttributeType::Index, {nullptr, indexCount * sizeof(uint32_t)});
        staticMesh->m_positionAttribute = CreateMeshAttribute(vertexCount, MeshAttributeType::Position, {nullptr, vertexCount * sizeof(glm::vec3)});
        staticMesh->m_normalAttribute   = CreateMeshAttribute(vertexCount, MeshAttributeType::Normal, {nullptr, vertexCount * sizeof(glm::vec3)});
        staticMesh->m_uvAttribute       = CreateMeshAttribute(vertexCount, MeshAttributeType::TexCoord, {nullptr, vertexCount * sizeof(glm::vec2)});
        return staticMesh;
    }

    StaticMeshLOD* UnifiedGeometryBufferPool::CreateStaticMeshLOD(
        TL::Span<const uint32_t>  indicies,
        TL::Span<const glm::vec3> positions,
        TL::Span<const glm::vec3> normals,
        TL::Span<const glm::vec2> uvs)
    {
        auto staticMesh = CreateStaticMeshLOD(positions.size(), indicies.size());

        // staticMesh->m_aabb.min;
        for (size_t i = 0; i < positions.size(); i++)
        {
            staticMesh->m_aabb.min.x = std::min(staticMesh->m_aabb.min.x, positions[i].x);
            staticMesh->m_aabb.min.y = std::min(staticMesh->m_aabb.min.y, positions[i].y);
            staticMesh->m_aabb.min.z = std::min(staticMesh->m_aabb.min.z, positions[i].z);
            staticMesh->m_aabb.max.x = std::max(staticMesh->m_aabb.max.x, positions[i].x);
            staticMesh->m_aabb.max.y = std::max(staticMesh->m_aabb.max.y, positions[i].y);
            staticMesh->m_aabb.max.z = std::max(staticMesh->m_aabb.max.z, positions[i].z);
        }

        WriteMeshAttribute(staticMesh->m_indexAttribute, 0, {(void*)indicies.data(), indicies.size_bytes()});
        WriteMeshAttribute(staticMesh->m_positionAttribute, 0, {(void*)positions.data(), positions.size_bytes()});
        WriteMeshAttribute(staticMesh->m_normalAttribute, 0, {(void*)normals.data(), normals.size_bytes()});
        WriteMeshAttribute(staticMesh->m_uvAttribute, 0, {(void*)uvs.data(), uvs.size_bytes()});

        return staticMesh;
    }

    void UnifiedGeometryBufferPool::ReleaseStaticMeshLOD(StaticMeshLOD* lod)
    {
        ReleaseMeshAttribute(lod->m_indexAttribute);
        ReleaseMeshAttribute(lod->m_positionAttribute);
        ReleaseMeshAttribute(lod->m_normalAttribute);
        ReleaseMeshAttribute(lod->m_uvAttribute);
        TL::Allocator::Destruct(lod);
    }

    MeshAttribute* UnifiedGeometryBufferPool::CreateMeshAttribute(U32 elementCount, MeshAttributeType type, TL::Block content)
    {
        auto  format     = kMeshAttributeFormat[U32(type)];
        auto& allocator  = m_allocators[U32(type)];
        auto  allocation = allocator.allocate(elementCount * RHI::GetFormatByteSize(format));
        if (allocation.offset == OffsetAllocator::Allocation::NO_SPACE)
        {
            TL_UNREACHABLE();
            return nullptr;
        }
        auto attribute = TL::Allocator::Construct<MeshAttribute>();
        attribute->m_type =    type;
        attribute->m_allocation = allocation;
        attribute->m_elementCount = elementCount;

        if (content.ptr)
        {
            WriteMeshAttribute(attribute, 0, content);
        }
        return attribute;
    }

    void UnifiedGeometryBufferPool::ReleaseMeshAttribute(MeshAttribute* attribute)
    {
        auto& allocator = m_allocators[U32(attribute->m_type)];
        allocator.free(attribute->m_allocation);
        TL::Allocator::Destruct(attribute);
    }

    void UnifiedGeometryBufferPool::WriteMeshAttribute(MeshAttribute* attribute, size_t offset, TL::Block content)
    {
        const auto& allocator = m_allocators[U32(attribute->m_type)];
        auto        meshSize  = allocator.allocationSize(attribute->m_allocation);
        TL_ASSERT(meshSize >= offset + content.size, "Overflow!");

        bool shouldUnamp = m_mappedPtr == nullptr;
        if (shouldUnamp)
        {
            m_mappedPtr = m_device->MapBuffer(m_buffer);
        }

        auto ptr = (char*)m_mappedPtr + m_segmentStartingOffsets[U32(attribute->m_type)] + attribute->m_allocation.offset + offset;
        ::memcpy(ptr, content.ptr, content.size);

        if (shouldUnamp)
        {
            m_device->UnmapBuffer(m_buffer);
            m_mappedPtr = nullptr;
        }
    }
} // namespace Engine