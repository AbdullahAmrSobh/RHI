#include "DrawList.hpp"
#include "Mesh.hpp"

namespace Engine
{
    ResultCode IndirectDrawList::Init(RHI::Device* device, uint32_t capacity)
    {
        ResultCode result;

        m_capacity = capacity;

        result = m_drawRequests.Init(*device, "draw-list", RHI::BufferUsage::Storage | RHI::BufferUsage::Indirect, capacity);

        result = m_uniforms.Init(*device, "draw-uniforms", RHI::BufferUsage::Vertex | RHI::BufferUsage::Storage, capacity);

        return ResultCode::Success;
    }

    void IndirectDrawList::Shutdown(RHI::Device* device)
    {
        m_drawRequests.Shutdown();
        m_uniforms.Shutdown();
    }

    void IndirectDrawList::AddStaticMesh(const StaticMeshLOD* staticMesh, GPU::MeshUniform uniform)
    {
        auto meshId    = staticMesh->GetGpuHandle().GetIndex();
        auto uniformId = m_uniforms.Insert(uniform).GetValue().GetIndex();

        GPU::DrawRequest drawRequest{
            .meshId    = meshId,
            .uniformId = uniformId,
            // .materialId = UINT32_MAX,
        };

        m_drawRequests.Insert(drawRequest).GetValue();
    }
} // namespace Engine