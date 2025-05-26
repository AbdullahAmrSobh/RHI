#include "CullPass.hpp"

#include "../PipelineLibrary.hpp"
#include "../Mesh.hpp"

namespace Engine
{
    ResultCode CullPass::Init(RHI::Device* device)
    {
        m_device = device;

        m_pipeline = PipelineLibrary::ptr->GetComputePipeline(ShaderNames::Cull);

        auto bindGroupLayout = PipelineLibrary::ptr->GetBindGroupLayout();
        m_bindGroup          = device->CreateBindGroup({.name = "Cull-BindGroup", .layout = bindGroupLayout});

        return ResultCode::Success;
    }

    void CullPass::Shutdown()
    {
    }

    void CullPass::AddPass(RHI::RenderGraph* rg, const IndirectDrawList& drawList)
    {
        this->m_drawIndirectArgs = rg->CreateBuffer("draw-indexed-indirect", drawList.m_capacity);

        rg->AddPass({
            .name          = "Cull",
            .type          = RHI::PassType::Compute,
            .setupCallback = [&](RHI::RenderGraphBuilder& builder)
            {
                this->m_drawIndirectArgs = builder.WriteBuffer(this->m_drawIndirectArgs, RHI::BufferUsage::Storage, RHI::PipelineStage::ComputeShader);
            },
            .executeCallback = [this, rg, drawList](RHI::CommandList& cmd)
            {
                auto& meshDrawData = GeometryBufferPool::ptr->m_drawParams;

                RHI::BindGroupBuffersUpdateInfo updateInfo[] = {
                    {BINDING_DRAWREQUESTS,        0, drawList.m_drawRequests.GetBindingInfo()                                 },
                    {BINDING_INDEXEDMESHES,       0, meshDrawData.GetBindingInfo()                                            },
                    {BINDING_DRAWPARAMETERSCOUNT, 0, RHI::BufferBindingInfo{rg->GetBufferHandle(this->m_drawIndirectArgs), 0} },
                    {BINDING_OUTDRAWPARAMETERS,   0, RHI::BufferBindingInfo{rg->GetBufferHandle(this->m_drawIndirectArgs), 64}},
                };
                m_device->UpdateBindGroup(m_bindGroup, {.buffers = updateInfo});

                cmd.BindComputePipeline(m_pipeline, {{m_bindGroup}});
                cmd.Dispatch({drawList.m_drawRequests.GetCount(), 1, 1});
            },
        });
    }
} // namespace Engine