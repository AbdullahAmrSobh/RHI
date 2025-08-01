
#include "Geometry.hpp"

namespace Engine
{
    static constexpr RHI::Format kMeshAttributeFormat[U32(MeshAttributeType::Count)] = {
        RHI::Format::R32_UINT,    // Index
        RHI::Format::RGB32_FLOAT, // Position
        RHI::Format::RGB32_FLOAT, // Normal
        RHI::Format::RG32_FLOAT,  // TexCoord
    };
    constexpr static U32 kVertexCount = 6400000; // 64k vertices

    GeometryBufferPool::GeometryBufferPool() = default;

    ResultCode GeometryBufferPool::Init(RHI::Device& device)
    {
        GeometryBufferPool::ptr = this;
        m_device                = &device;

        auto sizeIndex    = kVertexCount * sizeof(uint32_t);
        auto sizePosition = kVertexCount * sizeof(glm::vec3);
        auto sizeNormal   = kVertexCount * sizeof(glm::vec3);
        auto sizeTexCoord = kVertexCount * sizeof(glm::vec2);

        RHI::ResultCode result;

        result = m_bufferPools[U32(MeshAttributeType::Index)].Init(device, {.name = "ib", .hostMapped = true, .usageFlags = RHI::BufferUsage::Index, .byteSize = sizeIndex});
        if (RHI::IsError(result) == true)
            return result;

        result = m_bufferPools[U32(MeshAttributeType::Position)].Init(device, {.name = "vb-position", .hostMapped = true, .usageFlags = RHI::BufferUsage::Vertex, .byteSize = sizePosition});
        if (RHI::IsError(result) == true)
            return result;

        result = m_bufferPools[U32(MeshAttributeType::Normal)].Init(device, {.name = "vb-normal", .hostMapped = true, .usageFlags = RHI::BufferUsage::Vertex, .byteSize = sizeNormal});
        if (RHI::IsError(result) == true)
            return result;

        result = m_bufferPools[U32(MeshAttributeType::TexCoord)].Init(device, {.name = "vb-texCoord", .hostMapped = true, .usageFlags = RHI::BufferUsage::Vertex, .byteSize = sizeTexCoord});
        if (RHI::IsError(result) == true)
            return result;

        result = m_drawParams.Init(*m_device, "static-mesh-params", RHI::BufferUsage::Storage, 32);
        if (RHI::IsError(result) == true)
            return result;

        return result;
    }

    void GeometryBufferPool::Shutdown()
    {
        GeometryBufferPool::ptr = nullptr;

        m_drawParams.Shutdown();
        m_bufferPools[U32(MeshAttributeType::TexCoord)].Shutdown();
        m_bufferPools[U32(MeshAttributeType::Normal)].Shutdown();
        m_bufferPools[U32(MeshAttributeType::Position)].Shutdown();
        m_bufferPools[U32(MeshAttributeType::Index)].Shutdown();
    }

    RHI::BufferBindingInfo GeometryBufferPool::GetAttribute(MeshAttributeType attribute) const
    {
        auto buffer = m_bufferPools[U32(attribute)].GetBuffer();
        return {buffer, 0};
    }

    StaticMeshLOD* GeometryBufferPool::CreateStaticMeshLOD(U32 vertexCount, U32 indexCount)
    {
        auto staticMesh = TL::Construct<StaticMeshLOD>();

        staticMesh->m_indexAttribute    = CreateMeshAttribute(indexCount, MeshAttributeType::Index, {nullptr, indexCount * sizeof(uint32_t)});
        staticMesh->m_positionAttribute = CreateMeshAttribute(vertexCount, MeshAttributeType::Position, {nullptr, vertexCount * sizeof(glm::vec3)});
        staticMesh->m_normalAttribute   = CreateMeshAttribute(vertexCount, MeshAttributeType::Normal, {nullptr, vertexCount * sizeof(glm::vec3)});
        staticMesh->m_uvAttribute       = CreateMeshAttribute(vertexCount, MeshAttributeType::TexCoord, {nullptr, vertexCount * sizeof(glm::vec2)});
        staticMesh->m_indexCount        = indexCount;
        staticMesh->m_vertexCount       = vertexCount;
        staticMesh->m_indexOffset       = static_cast<uint32_t>(staticMesh->m_indexAttribute->m_allocation.offset / sizeof(uint32_t));
        staticMesh->m_vertexOffset      = I32(staticMesh->m_positionAttribute->m_allocation.offset / sizeof(glm::vec3));

        auto [indirectDrawArgs, result] = m_drawParams.Insert({
            .indexCount   = staticMesh->m_indexCount,
            .firstIndex   = staticMesh->m_indexOffset,
            .vertexOffset = I32(staticMesh->m_vertexOffset),
        });
        staticMesh->m_indirectDrawArgs  = indirectDrawArgs;

        return staticMesh;
    }

    StaticMeshLOD* GeometryBufferPool::CreateStaticMeshLOD(
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

    void GeometryBufferPool::ReleaseStaticMeshLOD(StaticMeshLOD* lod)
    {
        ReleaseMeshAttribute(lod->m_indexAttribute);
        ReleaseMeshAttribute(lod->m_positionAttribute);
        ReleaseMeshAttribute(lod->m_normalAttribute);
        ReleaseMeshAttribute(lod->m_uvAttribute);
        TL::Destruct(lod);
    }

    MeshAttribute* GeometryBufferPool::CreateMeshAttribute(U32 elementCount, MeshAttributeType type, TL::Block content)
    {
        auto format               = kMeshAttributeFormat[U32(type)];
        auto [allocation, result] = m_bufferPools[U32(type)].Allocate(elementCount * RHI::GetFormatByteSize(format), alignof(float));
        if (RHI::IsError(result))
        {
            TL_UNREACHABLE();
            return nullptr;
        }
        auto attribute            = TL::Construct<MeshAttribute>();
        attribute->m_type         = type;
        attribute->m_allocation   = allocation;
        attribute->m_elementCount = elementCount;

        if (content.ptr)
        {
            WriteMeshAttribute(attribute, 0, content);
        }
        return attribute;
    }

    void GeometryBufferPool::ReleaseMeshAttribute(MeshAttribute* attribute)
    {
        m_bufferPools[(U32)attribute->m_type].Release(attribute->m_allocation);
        TL::Destruct(attribute);
    }

    void GeometryBufferPool::WriteMeshAttribute(MeshAttribute* attribute, size_t offset, TL::Block content)
    {
        auto [bufferOffset, _] = attribute->GetAllocation();
        m_bufferPools[(U32)attribute->m_type].Write({U32(bufferOffset + offset), _}, content);
    }
} // namespace Engine