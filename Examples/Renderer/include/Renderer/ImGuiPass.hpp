#pragma once

#include <RenderCore/Resources.hpp>

#include <RHI/RHI.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/glm.hpp>

#include "Renderer/Common.hpp"
#include <Renderer-Shaders/ImGui.hpp>

namespace Engine
{
    class GraphicsShader;

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
        ImGuiContextHook m_newframeHook{};

        RHI::Device* m_device;

        uint32_t m_maxViewportsCount;

        TL::Ptr<GraphicsShader>  m_shader;
        RHI::BindGroup*        m_bindGroup;

        RHI::Sampler* m_sampler;
        RHI::Image*   m_image;

        Buffer<GPU::ImGuiShaderParam::CB> m_projectionCB;
        Buffer<ImDrawIdx>   m_indexBuffer;
        Buffer<ImDrawVert>  m_vertexBuffer;
    };
} // namespace Engine