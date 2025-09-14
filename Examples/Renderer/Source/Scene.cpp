#include "Renderer/Scene.hpp"

#include <Shaders/GpuCommonStructs.h>

#include <TL/Literals.hpp>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine
{

    void GpuSceneData::init(RHI::Device* device)
    {
        m_device = device;

        RHI::BufferCreateInfo m_geometryBuffersPoolCI{
            .name       = "geometry-buffers",
            .hostMapped = true,
            .usageFlags = RHI::BufferUsage::Vertex | RHI::BufferUsage::Index,
            .byteSize   = 1_gb,
        };
        m_geometryBuffersPool.init(device, m_geometryBuffersPoolCI);

        RHI::BufferCreateInfo m_constantBuffersPoolCI{
            .name       = "constant-buffers",
            .hostMapped = true,
            .usageFlags = RHI::BufferUsage::Uniform,
            .byteSize   = 1_mb,
        };
        m_constantBuffersPool.init(device, m_constantBuffersPoolCI);

        RHI::BufferCreateInfo m_structuredBuffersPoolCI{
            .name       = "structurd-buffers",
            .hostMapped = true,
            .usageFlags = RHI::BufferUsage::Storage,
            .byteSize   = 64_mb,
        };
        m_structuredBuffersPool.init(device, m_structuredBuffersPoolCI);

        m_transform  = createStructuredBuffer<glm::mat4x4>(m_structuredBuffersPool, k_transformMaxCapacity);
        m_meshLod    = createStructuredBuffer<GPU::SceneMeshLod>(m_structuredBuffersPool, k_meshLodMaxCapacity);
        m_light      = createStructuredBuffer<GPU::SceneLight>(m_structuredBuffersPool, k_lightMaxCapacity);
        m_renderable = createStructuredBuffer<GPU::SceneRenderable>(m_structuredBuffersPool, k_renderableMaxCapacity);

        // m_indexBuffer           = createStructuredBuffer<uint32_t>(m_geometryBuffersPool, k_maxIndexBufferElementsCount);
        // m_vertexBufferPositions = createStructuredBuffer<glm::vec3>(m_geometryBuffersPool, k_maxVertexBufferElementsCount);
        // m_vertexBufferNormals   = createStructuredBuffer<glm::vec3>(m_geometryBuffersPool, k_maxVertexBufferElementsCount);
        // m_vertexBufferTexcoords = createStructuredBuffer<glm::vec2>(m_geometryBuffersPool, k_maxVertexBufferElementsCount);
        // m_vertexBufferDrawIDs   = createStructuredBuffer<glm::vec2>(m_geometryBuffersPool, k_maxInstanceBufferElementsCount);
    }

    void GpuSceneData::shutdown()
    {
        freeStructuredBuffer(m_structuredBuffersPool, m_renderable);
        freeStructuredBuffer(m_structuredBuffersPool, m_light);
        freeStructuredBuffer(m_structuredBuffersPool, m_meshLod);
        freeStructuredBuffer(m_structuredBuffersPool, m_transform);
        m_structuredBuffersPool.shutdown();
        m_constantBuffersPool.shutdown();
        m_geometryBuffersPool.shutdown();
    }

    ResultCode Scene::Init()
    {
        auto& pool  = GpuSceneData::ptr->m_constantBuffersPool;
        m_sceneView = createConstantBuffer<GPU::SceneView>(pool);

        GPU::SceneView v{};
        v.viewToClipMatrix  = glm::identity<glm::mat4x4>();
        v.worldToViewMatrix = glm::identity<glm::mat4x4>();
        bufferWrite(pool, m_sceneView, v);

        return ResultCode::Success;
    }

    void Scene::Shutdown()
    {
        // auto& sbPool = GpuSceneData::ptr->m_structuredBuffersPool;
        // freeStructuredBuffer(sbPool, m_renderables);

        // auto& cbPool = GpuSceneData::ptr->m_constantBuffersPool;
        // freeConstantBuffer(cbPool, m_sceneCB);
    }
} // namespace Engine