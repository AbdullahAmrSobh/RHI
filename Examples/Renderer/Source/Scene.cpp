#include "Renderer/Scene.hpp"

#include <Shaders/GpuCommonStructs.h>

#include <TL/Literals.hpp>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{

    TL::Error GpuSceneData::init(RHI::Device* device)
    {
        m_device = device;

        m_constantBuffersPool = createConstantBufferPool(device, 1_mb);
        m_SBPoolRenderables   = createStructuredBufferPool<GPU::StaticMeshIndexed>(device, 4);
        m_indexPool           = createMeshBufferPool(device, sizeof(uint32_t) * 1024);
        m_vertexPoolPositions = createMeshBufferPool(device, sizeof(glm::vec3) * 1024);
        m_vertexPoolNormals   = createMeshBufferPool(device, sizeof(glm::vec3) * 1024);
        m_vertexPoolUVs       = createMeshBufferPool(device, sizeof(glm::vec2) * 1024);

        return TL::NoError;
    }

    void GpuSceneData::shutdown()
    {
        // freeMeshBufferPool(m_device, m_vertexPoolUVs);
        // freeMeshBufferPool(m_device, m_vertexPoolNormals);
        // freeMeshBufferPool(m_device, m_vertexPoolPositions);
        // freeMeshBufferPool(m_device, m_indexPool);
        // freeStructuredBufferPool(m_device, m_SBPoolRenderables);
        // freeConstantBufferPool(m_device, m_constantBuffersPool);
    }

    TL::Error Scene::init(RHI::Device* device)
    {
        auto& pool  = GpuSceneData::ptr->getConstantBuffersPool();
        m_sceneView = pool.allocate<GPU::SceneView>();

        GPU::SceneView v{};
        v.viewToClipMatrix  = glm::identity<glm::mat4x4>();
        v.worldToViewMatrix = glm::identity<glm::mat4x4>();
        pool.update(device, m_sceneView, v);

        return TL::NoError;
    }

    void Scene::shutdown(RHI::Device* m_device)
    {
        // auto& sbPool = GpuSceneData::ptr->m_structuredBuffersPool;
        // freeStructuredBuffer(sbPool, m_renderables);

        // auto& cbPool = GpuSceneData::ptr->m_constantBuffersPool;
        // freeConstantBuffer(cbPool, m_sceneCB);
    }
} // namespace Engine