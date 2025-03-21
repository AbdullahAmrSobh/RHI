#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "BufferPool.hpp"
#include "Common.hpp"
#include "Shaders/Public/GpuScene.h"

namespace Engine
{
    struct Transform;
    struct SceneViewDesc;

    class MaterialResource;
    class MeshResource;
    class ModelComponent;

    class LightResource;
    class LightComponent;

    class SceneView;

    class DrawList
    {
    };

    class DrawListMask
    {
    };

    class SceneManager
    {
    public:
        TL::Ptr<class Scene> CreateScene();

    private:
        TL::Ptr<class UnifiedGeometryBufferPool> m_meshBufferPool;
    };

    class SceneView
    {
    public:
    };

    class Scene
    {
    public:
        Scene();
        ~Scene();

        ResultCode Init();
        void       Shutdown();

        ModelComponent* DrawModel(const MeshResource& mesh, const MaterialResource& material, TL::Span<const Transform> transforms);
        void            RemoveModel(ModelComponent* model);

        LightComponent* CreateLight(const LightResource& lightResource);
        void            RemoveLight(LightComponent* light);

        SceneView* CreateSceneView(const SceneViewDesc& desc);
        void       DestroySceneView(SceneView* view);

        void ActivateSceneView(SceneView* view);
        void DeactivateSceneView(SceneView* view);

    private:
        GpuArray<glm::mat4> m_transforms;

        GpuArray<Shader::SpotLight> m_spotLights;

        GpuArray<Shader::PointLight> m_pointLights;

        GpuArray<Shader::DirectionalLight> m_directionalLights;
    };
} // namespace Engine