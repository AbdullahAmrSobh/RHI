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
        auto& getConstantBuffersPool()  { return m_constantBuffersPool; }

        auto& getSBPoolRenderables()  { return m_SBPoolRenderables; }

        auto& getIndexPool()  { return m_indexPool; }

        auto& getVertexPoolPositions()  { return m_vertexPoolPositions; }

        auto& getVertexPoolNormals()  { return m_vertexPoolNormals; }

        auto& getVertexPoolUVs()  { return m_vertexPoolUVs; }

    private:
        RHI::Device* m_device;

        ConstantBufferPool                           m_constantBuffersPool;
        StructuredBufferPool<GPU::StaticMeshIndexed> m_SBPoolRenderables;
        MeshBufferPool m_indexPool;
        MeshBufferPool m_vertexPoolPositions;
        MeshBufferPool m_vertexPoolNormals;
        MeshBufferPool m_vertexPoolUVs;
    };

    class Scene
    {
    public:
        Scene() = default;

        TL::Error init(RHI::Device* m_device);
        void      shutdown(RHI::Device* m_device);

        RHI::ImageSize2D               m_imageSize;
        ConstantBuffer<GPU::SceneView> m_sceneView;
    };
} // namespace Engine