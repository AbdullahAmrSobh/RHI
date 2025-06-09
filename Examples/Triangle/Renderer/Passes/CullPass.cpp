#include "CullPass.hpp"

#include "../PipelineLibrary.hpp"
#include "../Geometry.hpp"
#include "../Scene.hpp"

namespace Engine
{
    ResultCode CullPass::Init(RHI::Device* device)
    {
        m_device    = device;
        m_bindGroup = device->CreateBindGroup({.name = "Cull-BindGroup", .layout = PipelineLibrary::ptr->GetBindGroupLayout()});
        return ResultCode::Success;
    }

    void CullPass::Shutdown()
    {
    }

    void CullPass::AddPass(RHI::RenderGraph* rg, const Scene* scene)
    {
        this->m_drawIndirectArgs = rg->CreateBuffer("draw-indexed-indirect", kCapacity * sizeof(RHI::DrawIndexedParameters));

        rg->AddPass({
            .name          = "Cull",
            .type          = RHI::PassType::Compute,
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                this->m_drawIndirectArgs = builder.WriteBuffer(this->m_drawIndirectArgs, RHI::BufferUsage::Storage, RHI::PipelineStage::ComputeShader);
            },
            .executeCallback = [this, rg, scene](RHI::CommandList& cmd)
            {
                auto& meshDrawData = GeometryBufferPool::ptr->m_drawParams;

                RHI::BindGroupBuffersUpdateInfo updateInfo[] = {
                    {Bindings::DrawRequests,        0, scene->m_drawRequests.GetBindingInfo()                                   },
                    {Bindings::IndexedMeshes,       0, meshDrawData.GetBindingInfo()                                            },
                    {Bindings::DrawParametersCount, 0, RHI::BufferBindingInfo{rg->GetBufferHandle(this->m_drawIndirectArgs), 0} },
                    {Bindings::OutDrawParameters,   0, RHI::BufferBindingInfo{rg->GetBufferHandle(this->m_drawIndirectArgs), 64}},
                };
                m_device->UpdateBindGroup(m_bindGroup, {.buffers = updateInfo});

                auto pipeline = PipelineLibrary::ptr->GetComputePipeline(ShaderNames::Cull);

                cmd.BindComputePipeline(pipeline, {{m_bindGroup}});
                cmd.Dispatch({scene->m_drawRequests.GetCount(), 1, 1});
            },
        });
    }
} // namespace Engine