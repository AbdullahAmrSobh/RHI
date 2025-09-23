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
        m_SBPoolRenderables   = createStructuredBufferPool(device, 4_mb);
        m_indexPool           = createMeshBufferPool(device, sizeof(uint32_t) * 1024);
        m_vertexPoolPositions = createMeshBufferPool(device, sizeof(glm::vec3) * 1024);
        m_vertexPoolNormals   = createMeshBufferPool(device, sizeof(glm::vec3) * 1024);
        m_vertexPoolUVs       = createMeshBufferPool(device, sizeof(glm::vec2) * 1024);

        return TL::NoError;
    }

    void GpuSceneData::shutdown()
    {
        freeMeshBufferPool(m_device, m_vertexPoolUVs);
        freeMeshBufferPool(m_device, m_vertexPoolNormals);
        freeMeshBufferPool(m_device, m_vertexPoolPositions);
        freeMeshBufferPool(m_device, m_indexPool);
        freeStructuredBufferPool(m_device, m_SBPoolRenderables);
        freeConstantBufferPool(m_device, m_constantBuffersPool);
    }

    TL::Error Scene::init(RHI::Device* device)
    {
        auto& pool  = GpuSceneData::ptr->getConstantBuffersPool();
        m_sceneView = pool.allocate<GPU::SceneView>();

        GPU::SceneView v{};
        v.viewToClipMatrix  = glm::identity<glm::mat4x4>();
        v.worldToViewMatrix = glm::identity<glm::mat4x4>();
        pool.update(device, m_sceneView, v);

        // 4 vertices (quad in XY plane)
        static const std::array<glm::vec3, 4> positions = {
            glm::vec3(-0.5f, -0.5f, 0.0f), // bottom-left
            glm::vec3(0.5f, -0.5f, 0.0f),  // bottom-right
            glm::vec3(0.5f, 0.5f, 0.0f),   // top-right
            glm::vec3(-0.5f, 0.5f, 0.0f)   // top-left
        };

        // All normals facing +Z
        static const std::array<glm::vec3, 4> normals = {
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)};

        // Simple UVs
        static const std::array<glm::vec2, 4> texcoords = {
            glm::vec2(0.0f, 0.0f), // bottom-left
            glm::vec2(1.0f, 0.0f), // bottom-right
            glm::vec2(1.0f, 1.0f), // top-right
            glm::vec2(0.0f, 1.0f)  // top-left
        };

        // Two triangles
        static const std::array<uint32_t, 6> indices = {
            0, 1, 2,
            0, 2, 3,
        };

        m_mesh = StaticMeshLOD::create(
            TL::Span<const uint32_t>(indices.data(), indices.size()),
            TL::Span<const glm::vec3>(positions.data(), positions.size()),
            TL::Span<const glm::vec3>(normals.data(), normals.size()),
            TL::Span<const glm::vec2>(texcoords.data(), texcoords.size()));

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