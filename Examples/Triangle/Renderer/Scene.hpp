#pragma once

#include "Shaders/Public/GPU.h"

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "BufferPool.hpp"
#include "Common.hpp"

namespace Engine
{
    inline static constexpr uint32_t kCapacity = 1024 * 64 * 100;

    class SceneView;
    class MeshComponent;

    class Scene
    {
    public:
        SceneView* m_primaryView;

        uint32_t                   m_drawItemsCount;
        GpuArray<GPU::DrawRequest> m_drawRequests;
        GpuArray<glm::mat4x4>      m_transforms;

        ResultCode     Init(RHI::Device* device);
        void           Shutdown(RHI::Device* device);

        SceneView*     CreateView();
        void           DestroyView(SceneView* view);

        MeshComponent* AddStaticMesh(const class StaticMeshLOD* staticMesh, glm::mat4x4 transform);
        void           RemoveStaticMesh(MeshComponent* component);
    };

    class SceneView
    {
    public:
        ResultCode Init();
        void       Shutdown();

        TL::String                    m_name;
        UniformBuffer<GPU::SceneView> m_sceneViewUB;
    };
} // namespace Engine