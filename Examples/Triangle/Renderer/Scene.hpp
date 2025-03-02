#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "BufferPool.hpp"
#include "Shaders/Public/GpuScene.h"

namespace Engine
{
    class MeshResource;
    class ModelComponent;
    class SceneView;

    class Scene
    {
    public:
        ModelComponent* AddModel(MeshResource* mesh, const glm::mat4& transform);
        void            RemoveModel(ModelComponent* model);

        SceneView* CreateSceneView();
        void       DestroySceneView(SceneView* view);

    private:
        friend class Renderer;

        TL::Vector<TL::Ptr<ModelComponent>> m_models;
        TL::Vector<TL::Ptr<SceneView>>      m_views;

        SceneView* m_primarySceneView;

        // Gpu draw data
        // TODO: Remove this
        RHI::BufferBindingInfo               m_drawCountIndirectBuffer; // The number of draws to do
        GpuArray<RHI::DrawIndexedParameters> m_drawList;                // The draw list for all the models in the scene
        GpuArray<glm::mat3x4>                m_transforms;              // The transforms of all the models in the scene
    };
} // namespace Engine