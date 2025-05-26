#pragma once

#include <RHI/RHI.hpp>

#include "../Common.hpp"

namespace Engine
{
    class SceneView;

    class GBufferPass
    {
    public:
        // clang-format off
        static constexpr RHI::Format            FormatPosition  = RHI::Format::RGBA16_FLOAT;
        static constexpr RHI::ClearValue        ClearPosition   = {.f32{0.0f, 0.0f, 0.0f, 0.0f}};

        static constexpr RHI::Format            FormatNormal    = RHI::Format::RGBA16_FLOAT;
        static constexpr RHI::ClearValue        ClearNormal     = {.f32{0.0f, 0.0f, 1.0f, 0.0f}};

        static constexpr RHI::Format            FormatMaterial  = RHI::Format::RG16_FLOAT;
        static constexpr RHI::ClearValue        ClearMaterial   = {.f32{0u, 0u, 0u, 0u}};

        static constexpr RHI::Format            FormatDepth     = RHI::Format::D16;
        static constexpr RHI::DepthStencilValue ClearDepth      = {1.0f, 0};
        // clang-format on

        RHI::Handle<RHI::BindGroupLayout>  m_bindGroupLayout;
        RHI::Handle<RHI::PipelineLayout>   m_pipelineLayout;
        RHI::Handle<RHI::GraphicsPipeline> m_pipeline;
        RHI::Handle<RHI::BindGroup>        m_bindGroup;

        RHI::Handle<RHI::RGImage> m_position;
        RHI::Handle<RHI::RGImage> m_normal;
        RHI::Handle<RHI::RGImage> m_material;
        RHI::Handle<RHI::RGImage> m_depth;

        ResultCode Init(RHI::Device* device);
        void       Shutdown();

        void AddPass(RHI::RenderGraph* rg, const class CullPass& cullPass, TL::Function<void(RHI::CommandList&)> cb);
    };
} // namespace Engine