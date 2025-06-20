#pragma once

#include <RHI/RHI.hpp>

#include <imgui.h>

#include "../Common.hpp"

namespace Engine
{
    class Event;
}

namespace Engine
{
    class ImGuiPass
    {
    public:
        ImGuiPass() = default;

        ResultCode Init(RHI::Device* device, RHI::Format colorAttachmentFormat);
        void       Shutdown();

        bool Enabled() const  {  return true; }

        RHI::RGPass* AddPass(RHI::RenderGraph* renderGraph, RHI::RGImage*& outAttachment, ImDrawData* drawData);

    private:
        void InitGraphicsPipeline();
        bool UpdateBuffers(ImDrawData* drawData);

    private:
        RHI::Device* m_device;

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