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
        static constexpr RHI::Format Formats[] = {RHI::Format::RGBA16_FLOAT, RHI::Format::RGBA16_FLOAT, RHI::Format::RG16_FLOAT};
        static constexpr RHI::Format DepthFormat = RHI::Format::D16;
        // clang-format on

        RHI::Handle<RHI::BindGroup>  m_bindGroup;
        std::array<RHI::RGImage*, 4> m_attachments;

        ResultCode Init(RHI::Device* device);
        void       Shutdown(RHI::Device* device);
        void       AddPass(RHI::RenderGraph* rg, const class CullPass& cullPass, const class Scene* scene);
    };

    class LightingPass
    {
    public:
        RHI::Handle<RHI::BindGroup> m_bindGroup;
        RHI::RGImage*               m_attachment;

        ResultCode Init(RHI::Device* device);
        void       Shutdown(RHI::Device* device);
        void       AddPass(RHI::RenderGraph* rg, const GBufferPass& gbuffer, const class Scene* scene);
    };

    class ComposePass
    {
    public:
        RHI::Handle<RHI::BindGroup> m_bindGroup;

        ResultCode Init(RHI::Device* device);
        void       Shutdown(RHI::Device* device);
        void       AddPass(RHI::RenderGraph* rg, RHI::RGImage* input, RHI::RGImage*& output);
    };
} // namespace Engine