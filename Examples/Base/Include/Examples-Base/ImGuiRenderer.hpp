#pragma once

#include <imgui.h>

#include <RHI/RHI.hpp>

class IMGUI_IMPL_API ImGuiRenderer
{
public:
    bool Init(RHI::Context* context);
    void Shutdown();

    void NewFrame();
    void RenderDrawData(ImDrawData* draw_data, RHI::CommandList& commandList);

private:
    std::unique_ptr<RHI::ShaderModule> LoadVertexShaderModule();
    std::unique_ptr<RHI::ShaderModule> LoadPixelShaderModule();

private:
    RHI::Context* m_context;

    RHI::CommandList* m_currentCommandList;

    std::unique_ptr<RHI::BindGroupAllocator> m_bindGroupAllocator;

    RHI::Handle<RHI::BindGroupLayout> m_bindGroupLayout;

    RHI::Handle<RHI::BindGroup> m_bindGroup;

    RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;

    RHI::Handle<RHI::GraphicsPipeline> m_pipeline;

    std::unique_ptr<RHI::BufferPool> m_bufferPool;

    RHI::Handle<RHI::Buffer> m_vertexBuffer;

    RHI::Handle<RHI::Buffer> m_indexBuffer;

    RHI::Handle<RHI::Buffer> m_uniformBuffer;

    size_t m_vertexBufferSize;

    size_t m_indexBufferSize;
};
