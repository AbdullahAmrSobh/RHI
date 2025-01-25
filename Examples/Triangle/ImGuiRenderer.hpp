#pragma once

#include <RHI/RHI.hpp>

#include <imgui.h>

namespace Examples
{
    class Event;

    class IMGUI_IMPL_API ImGuiRenderer
    {
    public:
        ImGuiRenderer() = default;

        struct CreateInfo
        {
            RHI::Device* device;
            RHI::Format  renderTargetFormat;
        };

        void ProcessEvent(Event& event);

        void Init(const CreateInfo& createInfo);
        void Shutdown();

        void NewFrame();
        void RenderDrawData(ImDrawData* draw_data, RHI::CommandList& commandList);

    private:
        void InitGraphicsPipeline();
        void UpdateBuffers(ImDrawData* drawData);

    public:
        RHI::Device* m_context;

        ImGuiContext* m_imguiContext;

        RHI::Handle<RHI::BindGroup>        m_bindGroup;
        RHI::Handle<RHI::GraphicsPipeline> m_pipeline;

        RHI::Handle<RHI::Buffer>  m_uniformBuffer;
        RHI::Handle<RHI::Sampler> m_sampler;

        // TODO replace with RPI texture asset
        RHI::Handle<RHI::Image>     m_image;
        RHI::Handle<RHI::ImageView> m_imageView;

        // TODO: replace with RPI buffer stream
        size_t                   m_vertexBufferSize = 0;
        size_t                   m_indexBufferSize  = 0;
        RHI::Handle<RHI::Buffer> m_vertexBuffer;
        RHI::Handle<RHI::Buffer> m_indexBuffer;
    };
} // namespace Examples