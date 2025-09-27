#pragma once

#include "Renderer/Common.hpp"
#include "Renderer/Resources.hpp"
#include "Renderer/Geometry.hpp"

#include "Shaders/GpuCommonStructs.h"

namespace Engine
{
    class GpuSceneData : public Singleton<GpuSceneData>
    {
    public:
        GpuSceneData() = default;

        TL::Error init(RHI::Device* device);
        void      shutdown();

        TL::Ptr<StaticMeshLOD> createMesh(uint32_t elementsCount, uint32_t vertexCount);

    public:
        auto& getSBPoolRenderables() { return m_SBPoolRenderables; }

        auto& getIndexPool() { return m_indexPool; }
        auto& getVertexPoolPositions() { return m_vertexPoolPositions; }
        auto& getVertexPoolNormals() { return m_vertexPoolNormals; }
        auto& getVertexPoolUVs() { return m_vertexPoolUVs; }

        auto& getConstantBuffersPool() {return m_constantBuffersPool;}
        auto& getStructuredBuffersPool() {return m_structuredBuffersPool;}
        auto& getUnifiedGeometryBuffersPool() {return m_unifiedGeometryBuffersPool;}

    private:
        friend struct StaticMeshLOD;
        RHI::Device* m_device;

        ConstantBufferPool m_constantBuffersPool;
        BufferPool         m_structuredBuffersPool;
        BufferPool         m_unifiedGeometryBuffersPool;

        GPUArray<GPU::StaticMeshIndexed> m_SBPoolRenderables;
        GPUArray<uint32_t>               m_indexPool;
        GPUArray<glm::vec3>              m_vertexPoolPositions;
        GPUArray<glm::vec3>              m_vertexPoolNormals;
        GPUArray<glm::vec2>              m_vertexPoolUVs;
    };

    class SceneView;

    class Scene
    {
    public:
        Scene() = default;

        TL::Error init(RHI::Device* m_device);
        void      shutdown(RHI::Device* m_device);

        TL::Ptr<StaticMeshLOD> m_mesh;

        RHI::ImageSize2D       m_imageSize;
        Buffer<GPU::SceneView> m_sceneView;
    };
} // namespace Engine