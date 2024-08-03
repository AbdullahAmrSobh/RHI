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

        RHI::Handle<RHI::BindGroupLayout> m_bindGroupLayout;
        RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;
        RHI::Handle<RHI::GraphicsPipeline> m_pipeline;

        RHI::Handle<RHI::Image> m_image;
        RHI::Handle<RHI::ImageView> m_imageView;
        RHI::Handle<RHI::Sampler> m_sampler;

        size_t m_vertexBufferSize, m_indexBufferSize;
        RHI::Handle<RHI::Buffer> m_vertexBuffer;
        RHI::Handle<RHI::Buffer> m_indexBuffer;
        RHI::Handle<RHI::Buffer> m_uniformBuffer;
    };
} // namespace Examples