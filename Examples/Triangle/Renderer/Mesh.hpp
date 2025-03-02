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

namespace Engine
{
    /**
     * Unified geometry buffer archtecture where all meshes share same layout. data is stored in a single growing buffer.
     * Mesh attributes (position, normal, etc.) are stored non-interleaved in separate sections of the buffer.
     * Use GetAttributeBindingInfo to obtain the starting location of each attribute section. The buffer is partitioned
     * into MeshAttributeType::Count segments. Optional attributes (e.g. skinning data) are stored in the last segment.
     * Allocations from this buffer should never fail, as the buffer is grown as needed.
     */

    enum class MeshAttributeType
    {
        Index,
        Position,
        Normal,
        TexCoord,
        Count,
    };

    struct AABB
    {
        glm::vec3 min;
        glm::vec3 max;
    };

    class MeshAttribute
    {
    public:
        friend class UnifiedGeometryBufferPool;
        MeshAttribute()  = default;
        ~MeshAttribute() = default;

        MeshAttributeType GetType() const { return m_type; }

        U32 GetElementCount() const { return m_elementCount; }

        const Suballocation& GetAllocation() const { return m_allocation; }

    private:
        MeshAttributeType m_type;
        U32               m_elementCount;
        Suballocation     m_allocation;
    };

    class StaticMeshLOD
    {
    public:
        friend class UnifiedGeometryBufferPool;
        StaticMeshLOD()  = default;
        ~StaticMeshLOD() = default;

        U32 GetIndexCount() const { return m_indexCount; }

        U32 GetVertexCount() const { return m_vertexCount; }

        U32 GetIndexOffset() const { return m_indexOffset; }

        U32 GetVertexOffset() const { return m_vertexOffset; }

        const AABB& GetBoundingBox() const { return m_aabb; }

        const MeshAttribute* GetIndexAttribute() const { return m_indexAttribute; }

        const MeshAttribute* GetPositionAttribute() const { return m_positionAttribute; }

        const MeshAttribute* GetNormalAttribute() const { return m_normalAttribute; }

        const MeshAttribute* GetUVAttribute() const { return m_uvAttribute; }

    private:
        U32 m_indexCount;
        U32 m_vertexCount;
        U32 m_indexOffset;
        U32 m_vertexOffset;

        MeshAttribute* m_indexAttribute;
        MeshAttribute* m_positionAttribute;
        MeshAttribute* m_normalAttribute;
        MeshAttribute* m_uvAttribute;

        AABB m_aabb;
    };

    class UnifiedGeometryBufferPool
    {
    public:
        UnifiedGeometryBufferPool();
        UnifiedGeometryBufferPool(const UnifiedGeometryBufferPool&)            = delete;
        UnifiedGeometryBufferPool& operator=(const UnifiedGeometryBufferPool&) = delete;
        ~UnifiedGeometryBufferPool() = default;

        ResultCode Init(RHI::Device& device);
        void       Shutdown();

        RHI::BufferBindingInfo GetAttributeBindingInfo(MeshAttributeType attribute) const;

        void BeginUpdate();
        void EndUpdate();

        StaticMeshLOD* CreateStaticMeshLOD(U32 vertexCount, U32 indexCount);
        StaticMeshLOD* CreateStaticMeshLOD(
            TL::Span<const uint32_t>  indicies,
            TL::Span<const glm::vec3> positions,
            TL::Span<const glm::vec3> normals,
            TL::Span<const glm::vec2> uvs);
        void ReleaseStaticMeshLOD(StaticMeshLOD* lod);

    private:
        MeshAttribute* CreateMeshAttribute(U32 elementCount, MeshAttributeType type, TL::Block content);
        void           ReleaseMeshAttribute(MeshAttribute* attribute);
        void           WriteMeshAttribute(MeshAttribute* attribute, size_t offset, TL::Block content);

    private:
        RHI::Device*             m_device    = nullptr;
        /// The buffer containing all mesh data.
        RHI::Handle<RHI::Buffer> m_buffer    = RHI::NullHandle;
        /// The mapped pointer to the buffer.
        RHI::DeviceMemoryPtr     m_mappedPtr = nullptr;
        /// The starting offset for each attribute section.
        size_t                   m_segmentStartingOffsets[U32(MeshAttributeType::Count)];
        /// Use seperate allocator for each attribute section.
        /// @todo: Use a single allocator that allocate Vertex offset, and calcaulte byte offset based on that.
        Suballocator             m_allocators[U32(MeshAttributeType::Count)];
    };
} // namespace Engine