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
        auto& pool    = RenderContext::ptr->getUnifiedGeometryBuffersPool();

        RenderContext::ptr->getIndexPool().update(meshLod->m_indexBuffer, indcies);
        RenderContext::ptr->getVertexPoolPositions().update(meshLod->m_vbPositions, positions);
        RenderContext::ptr->getVertexPoolNormals().update(meshLod->m_vbNormals, normals);
        RenderContext::ptr->getVertexPoolUVs().update(meshLod->m_vbTexcoord, texcoord);
        return meshLod;
    }

    StaticMeshLOD::StaticMeshLOD(uint32_t indexCount, uint32_t elementsCount)
    {
        m_sbDrawArgs = RenderContext::ptr->getSBPoolRenderables().allocate(1);

        auto& mIndexPool           = RenderContext::ptr->getIndexPool();
        auto& mVertexPoolPositions = RenderContext::ptr->getVertexPoolPositions();
        auto& mVertexPoolNormals   = RenderContext::ptr->getVertexPoolNormals();
        auto& mVertexPoolUVs       = RenderContext::ptr->getVertexPoolUVs();

        m_indexBuffer = mIndexPool.allocate(indexCount);
        m_vbPositions = mVertexPoolPositions.allocate(elementsCount);
        m_vbNormals   = mVertexPoolNormals.allocate(elementsCount);
        m_vbTexcoord  = mVertexPoolUVs.allocate(elementsCount);

        m_drawArgs = GPU::StaticMeshIndexed{
            .indexCount   = m_indexBuffer.getElementsCount(),
            .firstIndex   = m_indexBuffer.getOffsetIndex(),
            .vertexOffset = (int32_t)m_vbPositions.getOffsetIndex(),
        };
        auto& pool = RenderContext::ptr->getStructuredBuffersPool();
        RenderContext::ptr->getSBPoolRenderables().update(m_sbDrawArgs, m_drawArgs);
    }

    StaticMeshLOD::~StaticMeshLOD()
    {
        RenderContext::ptr->getVertexPoolUVs().free(m_vbTexcoord);
        RenderContext::ptr->getVertexPoolNormals().free(m_vbNormals);
        RenderContext::ptr->getVertexPoolPositions().free(m_vbPositions);
        RenderContext::ptr->getIndexPool().free(m_indexBuffer);
        RenderContext::ptr->getSBPoolRenderables().free(m_sbDrawArgs);
    }
} // namespace Engine