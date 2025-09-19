#pragma once

#include <RHI/RHI.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/glm.hpp>

#include "Renderer/Common.hpp"
#include "Renderer/Resources.hpp"

namespace Engine
{
    class ImGuiPass
    {
    public:
        ImGuiPass() = default;

        TL::Error init(RHI::Device* device, RHI::Format colorAttachmentFormat, uint32_t maxViewportsCount = 8);
        void      shutdown();

        bool enabled() const { return true; }

        RHI::RGPass* addPass(RHI::RenderGraph* renderGraph, RHI::RGImage*& outAttachment, ImDrawData* drawData, uint32_t viewportID = 0);

    private:
        void initGraphicsPipeline();
        bool updateBuffers(ImDrawData* drawData);

    private:
        RHI::Device* m_device;

        uint32_t m_maxViewportsCount;

        RHI::PipelineLayout*   m_pipelineLayout;
        RHI::GraphicsPipeline* m_pipeline;

        ImGuiContextHook m_newframeHook{};

        RHI::Sampler*                      m_sampler;
        RHI::Image*                        m_image;
        DynamicConstantBuffer<glm::mat4x4> m_projectionCB;
        Buffer<ImDrawIdx>                  m_indexBuffer;
        Buffer<ImDrawVert>                 m_vertexBuffer;
        RHI::BindGroup*                    m_bindGroup;
    };
} // namespace Engine