#include "Geometry.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"

namespace Engine
{
    ResultCode Scene::Init(RHI::Device* device)
    {
        ResultCode result;
        result = m_drawRequests.Init(*device, "draw-requests", RHI::BufferUsage::Storage | RHI::BufferUsage::Indirect, kCapacity);
        result = m_transforms.Init(*device, "scene-transfroms", RHI::BufferUsage::Vertex | RHI::BufferUsage::Storage, kCapacity);
        return ResultCode::Success;
    }

    void Scene::Shutdown(RHI::Device* device)
    {
        m_drawRequests.Shutdown();
        m_transforms.Shutdown();
    }

    SceneView* Scene::CreateView()
    {
        auto                 view = TL::Allocator::Construct<SceneView>();
        TL_MAYBE_UNUSED auto res  = view->Init();
        m_primaryView             = view;
        return view;
    }

    void Scene::DestroyView(SceneView* view)
    {
        view->Shutdown();
        TL::Allocator::Destruct(view);
    }

    MeshComponent* Scene::AddStaticMesh(const StaticMeshLOD* staticMesh, glm::mat4x4 transform)
    {
        auto transformHandle = m_transforms.Insert(transform).GetValue();

        auto meshID = staticMesh->GetGpuHandle();

        GPU::DrawRequest request{};
        request.meshId    = meshID.GetIndex();
        request.uniformId = transformHandle.GetIndex();
        auto r            = m_drawRequests.Insert(request);

        return nullptr;
    }

    void Scene::RemoveStaticMesh(MeshComponent* component)
    {
        // noop for now
    }

    ResultCode SceneView::Init()
    {
        m_sceneViewUB = Renderer::ptr->m_allocators.uniformPool.AllocateUniformBuffer<GPU::SceneView>();

        return ResultCode::Success;
    }

    void SceneView::Shutdown()
    {
        // Renderer::ptr->m_allocators.uniformPool.Release(m_sceneViewUB);
    }

} // namespace Engine