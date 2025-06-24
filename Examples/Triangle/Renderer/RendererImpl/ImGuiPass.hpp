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

        ResultCode Init(RHI::Device* device, RHI::Format colorAttachmentFormat, uint32_t maxViewportsCount = 8);
        void       Shutdown();

        bool Enabled() const  {  return true; }

        RHI::RGPass* AddPass(RHI::RenderGraph* renderGraph, RHI::RGImage*& outAttachment, ImDrawData* drawData, uint32_t viewportID = 0);

    private:
        void InitGraphicsPipeline();
        bool UpdateBuffers(ImDrawData* drawData);

    private:
        RHI::Device* m_device;

        uint32_t m_maxViewportsCount;

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