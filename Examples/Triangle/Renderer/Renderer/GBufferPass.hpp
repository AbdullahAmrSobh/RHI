#pragma once

#include <RHI/RHI.hpp>

#include <slang/slang.h>

namespace Engine
{
    class Shader
    {
    public:
        RHI::Handle<RHI::BindGroupLayout>  m_bindGroupLayout[4];
        RHI::Handle<RHI::BindGroup>        m_bindGroup[4];
        RHI::Handle<RHI::PipelineLayout>   m_pipelineLayout;
        RHI::Handle<RHI::GraphicsPipeline> m_pipeline;

        void Init(RHI::Device& device, const char* path);
    };

    class GBufferPass
    {
    public:
        static constexpr auto FormatPosition = RHI::Format::RGB32_FLOAT;
        static constexpr auto ClearPosition = RHI::ClearValue{};

        static constexpr auto FormatNormal = RHI::Format::RGBA16_FLOAT;
        static constexpr auto ClearNormal = RHI::ClearValue{};

        static constexpr auto FormatMaterial = RHI::Format::R16_UINT;
        static constexpr auto ClearMaterial = RHI::ClearValue{};

        static constexpr auto FormatDepth = RHI::Format::D16;
        static constexpr auto ClearDepth = RHI::DepthStencilValue{};

        Shader* m_shader;

        RHI::Handle<RHI::RGImage> m_position;
        RHI::Handle<RHI::RGImage> m_normal;
        RHI::Handle<RHI::RGImage> m_material;
        RHI::Handle<RHI::RGImage> m_depth;

        void Init(RHI::Device& device);
        void Shutdown();

        void AddPass(RHI::RenderGraph& rg);
    };
} // namespace Engine