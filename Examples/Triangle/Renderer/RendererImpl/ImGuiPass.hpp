#pragma once

#include <RHI/RHI.hpp>

#include <imgui.h>
#include <imgui_internal.h>

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

        bool Enabled() const { return true; }

        RHI::RGPass* AddPass(RHI::RenderGraph* renderGraph, RHI::RGImage*& outAttachment, ImDrawData* drawData, uint32_t viewportID = 0);

    private:
        void InitGraphicsPipeline();
        bool UpdateBuffers(ImDrawData* drawData);

    private:
        RHI::Device* m_device;

        uint32_t m_maxViewportsCount;

        RHI::BindGroup*        m_bindGroup;
        RHI::PipelineLayout*   m_pipelineLayout;
        RHI::GraphicsPipeline* m_pipeline;

        RHI::Buffer*  m_uniformBuffer;
        RHI::Sampler* m_sampler;
        RHI::Image*   m_image;

        ImGuiContextHook         m_newframeHook{};
        size_t                   m_vertexBufferSize   = 0;
        size_t                   m_indexBufferSize    = 0;
        size_t                   m_vertexBufferOffset = 0;
        size_t                   m_indexBufferOffset  = 0;
        RHI::Buffer* m_vertexBuffer;
        RHI::Buffer* m_indexBuffer;
    };
} // namespace Engine