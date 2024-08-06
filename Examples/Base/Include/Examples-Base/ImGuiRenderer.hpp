#pragma once

#include <imgui.h>

#include <RHI/RHI.hpp>

namespace Examples
{
    class Event;

    class IMGUI_IMPL_API ImGuiRenderer
    {
    public:
        struct CreateInfo
        {
            RHI::Context* context;
            TL::Vector<uint32_t> shaderBlob;
            RHI::Format renderTargetFormat;
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
        RHI::Context* m_context;

        ImGuiContext* m_imguiContext;

        RHI::Handle<RHI::BindGroup> m_bindGroup;
        RHI::Handle<RHI::GraphicsPipeline> m_pipeline;

        RHI::Handle<RHI::Buffer> m_uniformBuffer;
        RHI::Handle<RHI::Sampler> m_sampler;

        // TODO replace with RPI texture asset
        RHI::Handle<RHI::Image> m_image;
        RHI::Handle<RHI::ImageView> m_imageView;


        // TODO: replace with RPI buffer stream
        size_t m_vertexBufferSize, m_indexBufferSize;
        RHI::Handle<RHI::Buffer> m_vertexBuffer;
        RHI::Handle<RHI::Buffer> m_indexBuffer;
    };
} // namespace Examples