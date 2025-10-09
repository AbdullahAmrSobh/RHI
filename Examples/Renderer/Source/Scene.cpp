#include "Renderer/Scene.hpp"

#include <Shaders/GpuCommonStructs.h>

#include <TL/Literals.hpp>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{
    TL::Error RenderContext::init(RHI::Device* device)
    {
        m_device = device;

        m_constantBuffersPool.init(device, "constant-buffers", RHI::BufferUsage::Uniform, 4_mb);
        m_structuredBuffersPool.init(device, "structured-buffers", RHI::BufferUsage::Storage, 4_mb);
        m_unifiedGeometryBuffersPool.init(device, "unified-geometry-buffers", RHI::BufferUsage::VertexIndex, 2_gb);

        m_SBPoolRenderables.init(m_structuredBuffersPool, 1000);
        m_indexPool.init(m_unifiedGeometryBuffersPool, 15625000);
        m_vertexPoolPositions.init(m_unifiedGeometryBuffersPool, 15625000);
        m_vertexPoolNormals.init(m_unifiedGeometryBuffersPool, 15625000);
        m_vertexPoolUVs.init(m_unifiedGeometryBuffersPool, 15625000);

        return TL::NoError;
    }

    void RenderContext::shutdown()
    {
        m_SBPoolRenderables.shutdown();
        m_vertexPoolUVs.shutdown();
        m_vertexPoolNormals.shutdown();
        m_vertexPoolPositions.shutdown();
        m_indexPool.shutdown();

        m_constantBuffersPool.shutdown();
    }

    Scene::Scene()
        : m_drawList(2048)
    {
        auto device = RenderContext::ptr->m_device;

        auto& pool  = RenderContext::ptr->getConstantBuffersPool();
        m_sceneView = pool.allocate<GPU::SceneView>(1, device->GetLimits().minUniformBufferOffsetAlignment);

        // m_lights.init(RenderContext::ptr->getStructuredBuffersPool(), 128);

        GPU::SceneView v{
            .worldToViewMatrix = glm::identity<glm::mat4x4>(),
            .viewToClipMatrix  = glm::identity<glm::mat4x4>(),
        };
        pool.update(m_sceneView, v);
    }

    Scene::~Scene()
    {
        // auto& sbPool = RenderContext::ptr->m_structuredBuffersPool;
        // freeStructuredBuffer(sbPool, m_renderables);

        // auto& cbPool = RenderContext::ptr->m_constantBuffersPool;
        // freeConstantBuffer(cbPool, m_sceneCB);
    }

    void Scene::addMesh(const StaticMeshLOD* mesh,
        const Material*                      material,
        glm::mat4x4                          transform,
        uint32_t                             viewMask)
    {
        m_drawList.push(mesh, material, transform, viewMask);
    }

    // void Scene::addLight(GPU::DirectionalLight dirLight)
    // {
    //     auto light = m_lights.allocate(1);
    //     m_lights.update(light, dirLight);
    // }

} // namespace Engine