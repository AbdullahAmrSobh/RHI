#pragma once

#include <RHI/RHI.hpp>

#include "ImGuiPass.hpp"
#include "../Common.hpp"

namespace Engine
{
    struct CullPass
    {
        RHI::Handle<RHI::BindGroup> m_bindGroup;
        RHI::RGBuffer*              m_drawIndirectArgs;

        ResultCode Init(RHI::Device* device);
        void       Shutdown(RHI::Device* device);
        void       AddPass(RHI::Device* device, RHI::RenderGraph* rg, const class Scene* scene);
    };

    struct GBufferPass
    {
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

    struct LightingPass
    {
        RHI::Handle<RHI::BindGroup> m_bindGroup;
        RHI::RGImage*               m_attachment;

        ResultCode Init(RHI::Device* device);
        void       Shutdown(RHI::Device* device);
        void       AddPass(RHI::RenderGraph* rg, const GBufferPass& gbuffer, const class Scene* scene);
    };

    struct ComposePass
    {
        RHI::Handle<RHI::BindGroup> m_bindGroup;

        ResultCode Init(RHI::Device* device);
        void       Shutdown(RHI::Device* device);
        void       AddPass(RHI::RenderGraph* rg, RHI::RGImage* input, RHI::RGImage*& output);
    };

    class DeferredRenderer
    {
    public:
        // Passes
        CullPass     m_cullPass;
        GBufferPass  m_gbufferPass;
        LightingPass m_lightingPass;
        ComposePass  m_composePass;
        ImGuiPass    m_imguiPass;

        ResultCode Init(RHI::Device* device);
        void       Shutdown(RHI::Device* device);

        void Render(RHI::Device* device, RHI::RenderGraph* rg, const Scene* scene, RHI::RGImage* outputAttachment);
    };
} // namespace Engine