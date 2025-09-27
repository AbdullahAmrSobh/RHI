#include "Renderer/Geometry.hpp"
#include "Renderer/Scene.hpp"

namespace Engine
{
    TL::Ptr<StaticMeshLOD> StaticMeshLOD::create(uint32_t ibSize, uint32_t vbSize)
    {
        return TL::CreatePtr<StaticMeshLOD>(ibSize, vbSize);
    }

    TL::Ptr<StaticMeshLOD> StaticMeshLOD::create(
        TL::Span<const uint32_t>  indcies,
        TL::Span<const glm::vec3> positions,
        TL::Span<const glm::vec3> normals,
        TL::Span<const glm::vec2> texcoord)
    {
        TL_ASSERT(positions.size() == normals.size());
        TL_ASSERT(positions.size() == texcoord.size());
        auto  meshLod = create(indcies.size(), positions.size());
        auto& pool    = GpuSceneData::ptr->getUnifiedGeometryBuffersPool();

        pool.update(GpuSceneData::ptr->m_device, meshLod->m_indexBuffer.getBuffer(), meshLod->m_indexBuffer.getOffsetElements(), indcies);
        pool.update(GpuSceneData::ptr->m_device, meshLod->m_vbPositions.getBuffer(), meshLod->m_vbPositions.getOffsetElements(), positions);
        pool.update(GpuSceneData::ptr->m_device, meshLod->m_vbNormals.getBuffer(), meshLod->m_vbNormals.getOffsetElements(), normals);
        pool.update(GpuSceneData::ptr->m_device, meshLod->m_vbTexcoord.getBuffer(), meshLod->m_vbTexcoord.getOffsetElements(), texcoord);
        return meshLod;
    }

    StaticMeshLOD::StaticMeshLOD(uint32_t indexCount, uint32_t elementsCount)
    {
        m_sbDrawArgs  = GpuSceneData::ptr->getSBPoolRenderables().allocate(1);
        m_indexBuffer = GpuSceneData::ptr->getIndexPool().allocate(indexCount);
        m_vbPositions = GpuSceneData::ptr->getVertexPoolPositions().allocate(elementsCount);
        m_vbNormals   = GpuSceneData::ptr->getVertexPoolNormals().allocate(elementsCount);
        m_vbTexcoord  = GpuSceneData::ptr->getVertexPoolUVs().allocate(elementsCount);

        m_drawArgs = GPU::StaticMeshIndexed{
            .indexCount   = m_indexBuffer.getCount(),
            .firstIndex   = m_indexBuffer.getOffsetElements(),
            .vertexOffset = (int32_t)m_vbPositions.getOffsetElements(),
        };
        auto& pool = GpuSceneData::ptr->getStructuredBuffersPool();
        pool.update(GpuSceneData::ptr->m_device, m_sbDrawArgs.getBuffer(), m_sbDrawArgs.getOffsetElements(), TL::Span<const GPU::StaticMeshIndexed>{m_drawArgs});
    }

    StaticMeshLOD::~StaticMeshLOD()
    {
        GpuSceneData::ptr->getVertexPoolUVs().free(m_vbTexcoord);
        GpuSceneData::ptr->getVertexPoolNormals().free(m_vbNormals);
        GpuSceneData::ptr->getVertexPoolPositions().free(m_vbPositions);
        GpuSceneData::ptr->getIndexPool().free(m_indexBuffer);
        GpuSceneData::ptr->getSBPoolRenderables().free(m_sbDrawArgs);
    }
} // namespace Engine