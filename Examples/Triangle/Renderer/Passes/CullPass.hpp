#pragma once

#include <RHI/RHI.hpp>

#include "../Common.hpp"

namespace Engine
{
    class SceneView;

    class CullPass
    {
    public:
        RHI::Handle<RHI::BindGroupLayout>  m_bindGroupLayout;
        RHI::Handle<RHI::PipelineLayout>   m_pipelineLayout;
        RHI::Handle<RHI::ComputePipeline>  m_pipeline;
        RHI::Handle<RHI::BindGroup>        m_bindGroup;

        RHI::RGBuffer* m_drawIndirectArgs;

        RHI::Device* m_device;

        ResultCode Init(RHI::Device* device);
        void       Shutdown();

        void AddPass(RHI::RenderGraph* rg, const class Scene* scene);
    };
} // namespace Engine