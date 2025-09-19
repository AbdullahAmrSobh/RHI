#pragma once

#include "Renderer/Common.hpp"
#include "Renderer/Resources.hpp"

#include "Shaders/GpuCommonStructs.h"

namespace Engine
{

    class GpuSceneData : public Singleton<GpuSceneData>
    {
    public:
        GpuSceneData() = default;

        TL::Error init(RHI::Device* device);
        void      shutdown();

    public:
        RHI::Device* m_device;

        BufferPool m_constantBuffersPool;

        static constexpr int k_transformMaxCapacity  = 1000;
        static constexpr int k_meshLodMaxCapacity    = 1000;
        static constexpr int k_lightMaxCapacity      = 1000;
        static constexpr int k_renderableMaxCapacity = 1000;

        BufferPool                             m_structuredBuffersPool;
        StructuredBuffer<glm::mat4x4>          m_transform;
        StructuredBuffer<GPU::SceneMeshLod>    m_meshLod;
        StructuredBuffer<GPU::SceneLight>      m_light;
        StructuredBuffer<GPU::SceneRenderable> m_renderable;

        static constexpr int k_maxIndexBufferElementsCount    = 1000;
        static constexpr int k_maxVertexBufferElementsCount   = 1000;
        static constexpr int k_maxInstanceBufferElementsCount = 1000;

        // vertex-buffers
        BufferPool         m_geometryBuffersPool;
        Buffer<uint32_t>   m_indexBuffer           = m_geometryBuffersPool.allocate<uint32_t>(k_maxIndexBufferElementsCount);
        Buffer<glm::vec3>  m_vertexBufferPositions = m_geometryBuffersPool.allocate<glm::vec3>(k_maxVertexBufferElementsCount);
        Buffer<glm::vec3>  m_vertexBufferNormals   = m_geometryBuffersPool.allocate<glm::vec3>(k_maxVertexBufferElementsCount);
        Buffer<glm::vec2>  m_vertexBufferTexcoords = m_geometryBuffersPool.allocate<glm::vec2>(k_maxVertexBufferElementsCount);
        Buffer<glm::ivec2> m_vertexBufferDrawIDs   = m_geometryBuffersPool.allocate<glm::ivec2>(k_maxInstanceBufferElementsCount);
    };

    class Scene
    {
    public:
        Scene() = default;

        TL::Error init();
        void      shutdown();

        ConstantBuffer<GPU::SceneView> m_sceneView;
    };
} // namespace Engine