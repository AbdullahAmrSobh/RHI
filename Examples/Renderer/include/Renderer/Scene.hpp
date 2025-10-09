#pragma once

#include "Renderer/Common.hpp"
#include "Renderer/Resources.hpp"
#include "Renderer/MeshDrawProcessor.hpp"
#include "Renderer/Geometry.hpp"

#include "Shaders/GpuCommonStructs.h"

namespace Engine
{
    class RenderContext : public Singleton<RenderContext>
    {
    public:
        // GpuSceneData() = default;

        TL::Error init(RHI::Device* device);
        void      shutdown();

        TL::Ptr<StaticMeshLOD> createMesh(uint32_t elementsCount, uint32_t vertexCount);

        RHI::Device* getDevice() const { return m_device; }

        template<typename T>
        Buffer<T> allocateConstantBuffer(uint32_t count = 1)
        {
            auto alignment = m_device->GetLimits().minUniformBufferOffsetAlignment;
            return m_constantBuffersPool.allocate<T>(count, alignment);
        }

        template<typename T>
        void freeConstantBuffer(Buffer<T>& buffer)
        {
            m_constantBuffersPool.free(buffer);
            buffer = {};
        }

        template<typename T>
        Buffer<T> allocateStructuredBuffer(uint32_t count = 1)
        {
            auto alignment = m_device->GetLimits().minStorageBufferOffsetAlignment;
            return m_structuredBuffersPool.allocate<T>(count, alignment);
        }

        template<typename T>
        void freeStructuredBuffer(Buffer<T>& buffer)
        {
            m_structuredBuffersPool.free(buffer);
            buffer = {};
        }

    public:
        auto& getSBPoolRenderables() { return m_SBPoolRenderables; }

        auto& getIndexPool() { return m_indexPool; }

        auto& getVertexPoolPositions() { return m_vertexPoolPositions; }

        auto& getVertexPoolNormals() { return m_vertexPoolNormals; }

        auto& getVertexPoolUVs() { return m_vertexPoolUVs; }

        auto& getConstantBuffersPool() { return m_constantBuffersPool; }

        auto& getStructuredBuffersPool() { return m_structuredBuffersPool; }

        auto& getUnifiedGeometryBuffersPool() { return m_unifiedGeometryBuffersPool; }

        // private:
        friend struct StaticMeshLOD;
        RHI::Device* m_device;

        BufferPool m_constantBuffersPool;
        BufferPool m_structuredBuffersPool;
        BufferPool m_unifiedGeometryBuffersPool;

        GPUArray<GPU::StaticMeshIndexed> m_SBPoolRenderables;
        GPUArray<uint32_t>               m_indexPool;
        GPUArray<glm::vec3>              m_vertexPoolPositions;
        GPUArray<glm::vec3>              m_vertexPoolNormals;
        GPUArray<glm::vec2>              m_vertexPoolUVs;
    };

    // template<typename T>
    // class SceneComponent
    // {
    // public:
    // };

    // class MeshComponent final : public SceneComponent<MeshComponent>
    // {
    // public:
    //     void updateTransform(const glm::mat4x4& transform);
    // };

    // class LightComponent final : public SceneComponent<MeshComponent>
    // {
    // public:
    // };

    // class SceneView final : public SceneComponent<SceneView>
    // {
    // public:
    //     SceneView();
    //     ~SceneView();

    //     void setPosition(const glm::vec3& position);
    //     void setDirection(const glm::vec3& direction);
    //     void setUp(const glm::vec3& up);
    //     void setRight(const glm::vec3& right);
    //     void setViewMatrix(const glm::mat4x4& view);
    //     void setProjMatrix(const glm::mat4x4& proj);

    //     void onUpdate(RHI::RenderGraph* rg);

    //     const Buffer<GPU::SceneView>& getConstantBuffer() const;
    // };

    class Scene
    {
    public:
        Scene();
        ~Scene();

        void addMesh(const StaticMeshLOD* mesh,
            const Material*               material,
            glm::mat4x4                   transform,
            uint32_t                      viewMask = 0x1u);

        RHI::ImageSize2D       m_imageSize;
        Buffer<GPU::SceneView> m_sceneView;

        DrawList m_drawList;
    };
} // namespace Engine