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
        auto meshLod = create(indcies.size(), positions.size());
        GpuSceneData::ptr->getIndexPool().update(GpuSceneData::ptr->m_device, meshLod->m_indexBuffer, 0, indcies);
        GpuSceneData::ptr->getVertexPoolPositions().update(GpuSceneData::ptr->m_device, meshLod->m_vbPositions, 0, positions);
        GpuSceneData::ptr->getVertexPoolNormals().update(GpuSceneData::ptr->m_device, meshLod->m_vbNormals, 0, normals);
        GpuSceneData::ptr->getVertexPoolUVs().update(GpuSceneData::ptr->m_device, meshLod->m_vbTexcoord, 0, texcoord);
        return meshLod;
    }

    StaticMeshLOD::StaticMeshLOD(uint32_t indexCount, uint32_t elementsCount)
    {
        m_sbDrawArgs  = GpuSceneData::ptr->getSBPoolRenderables().allocate<GPU::StaticMeshIndexed>(1);
        m_indexBuffer = GpuSceneData::ptr->getIndexPool().allocate<uint32_t>(indexCount);
        m_vbPositions = GpuSceneData::ptr->getVertexPoolPositions().allocate<glm::vec3>(elementsCount);
        m_vbNormals   = GpuSceneData::ptr->getVertexPoolNormals().allocate<glm::vec3>(elementsCount);
        m_vbTexcoord  = GpuSceneData::ptr->getVertexPoolUVs().allocate<glm::vec2>(elementsCount);

        m_drawArgs = GPU::StaticMeshIndexed{
            .indexCount   = m_indexBuffer.getCount(),
            .firstIndex   = (uint32_t)m_indexBuffer.getOffset() / (uint32_t)sizeof(uint32_t),
            .vertexOffset = (int32_t)m_vbPositions.getOffset() / (int32_t)sizeof(glm::vec3),
        };
        GpuSceneData::ptr->getSBPoolRenderables().update<GPU::StaticMeshIndexed>(GpuSceneData::ptr->m_device, m_sbDrawArgs, 0, m_drawArgs);
    }

    StaticMeshLOD::~StaticMeshLOD()
    {
        GpuSceneData::ptr->getVertexPoolUVs().free<glm::vec2>(m_vbTexcoord);
        GpuSceneData::ptr->getVertexPoolNormals().free<glm::vec3>(m_vbNormals);
        GpuSceneData::ptr->getVertexPoolPositions().free<glm::vec3>(m_vbPositions);
        GpuSceneData::ptr->getIndexPool().free<uint32_t>(m_indexBuffer);
        GpuSceneData::ptr->getSBPoolRenderables().free<GPU::StaticMeshIndexed>(m_sbDrawArgs);
    }
} // namespace Engine