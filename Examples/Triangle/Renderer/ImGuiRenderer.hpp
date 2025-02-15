#pragma once

#include <RHI/RHI.hpp>

#include <imgui.h>

#include "Common.hpp"

namespace Examples
{
    class Event;
}

namespace Engine
{
    class IMGUI_IMPL_API ImGuiRenderer
    {
    public:
        ImGuiRenderer() = default;

        void ProcessEvent(Examples::Event& event);

        ResultCode Init(RHI::Device* device, RHI::Format colorAttachmentFormat);
        void       Shutdown();

        void NewFrame();
        void RenderDrawData(ImDrawData* draw_data, RHI::CommandList& commandList);

    private:
        void InitGraphicsPipeline();
        void UpdateBuffers(ImDrawData* drawData);

    private:
        RHI::Device* m_device;

        ImGuiContext* m_imguiContext;

        RHI::Handle<RHI::BindGroup>        m_bindGroup;
        RHI::Handle<RHI::PipelineLayout>   m_pipelineLayout;
        RHI::Handle<RHI::GraphicsPipeline> m_pipeline;

        RHI::Handle<RHI::Buffer>  m_uniformBuffer;
        RHI::Handle<RHI::Sampler> m_sampler;
        RHI::Handle<RHI::Image>   m_image;

        size_t                   m_vertexBufferSize = 0;
        size_t                   m_indexBufferSize  = 0;
        RHI::Handle<RHI::Buffer> m_vertexBuffer;
        RHI::Handle<RHI::Buffer> m_indexBuffer;
    };
} // namespace Engine