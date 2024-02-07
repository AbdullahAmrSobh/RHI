#pragma once

#include <imgui.h>

#include <RHI/RHI.hpp>

#include <GLFW/glfw3.h>

class IMGUI_IMPL_API ImGuiRenderer
{
public:
    void Init(RHI::Context* context, RHI::FrameScheduler* scheduler, RHI::CommandListAllocator* commandListAllocator, RHI::BindGroupAllocator* bindGroupAllocator, RHI::ImagePool& imagePool, RHI::BufferPool& bufferPool, const std::vector<uint32_t>& shaderModuleBlob);
    void Shutdown();

    void NewFrame();
    void RenderDrawData(ImDrawData* draw_data, RHI::CommandList& commandList);

    void InstallGlfwCallbacks(GLFWwindow* window);

private:
    void InitGraphicsPipeline();
    void UpdateBuffers(ImDrawData* drawData);

public:
    RHI::Context* m_context;

    ImGuiContext* m_imguiContext;

    RHI::BindGroupAllocator* m_bindGroupAllocator;
    RHI::Handle<RHI::BindGroup> m_bindGroup;

    RHI::Handle<RHI::BindGroupLayout> m_bindGroupLayout;
    RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline> m_pipeline;

    RHI::ImagePool* m_imagePool;
    RHI::Handle<RHI::Image> m_image;
    RHI::Handle<RHI::ImageView> m_imageView;
    RHI::Handle<RHI::Sampler> m_sampler;

    RHI::BufferPool* m_bufferPool; // pool used to allocate resources
    RHI::Handle<RHI::Buffer> m_vertexBuffer;
    RHI::Handle<RHI::Buffer> m_indexBuffer;
    RHI::Handle<RHI::Buffer> m_uniformBuffer;

    GLFWwindow*             m_window;
    double                  m_time;
    GLFWwindow*             m_mouseWindow;
    GLFWcursor*             m_mouseCursors[ImGuiMouseCursor_COUNT];
    ImVec2                  m_lastValidMousePos;
    bool                    m_installedCallbacks;
    bool                    m_callbacksChainForAllWindows;
    // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
    GLFWwindowfocusfun      m_prevUserCallbackWindowFocus;
    GLFWcursorposfun        m_prevUserCallbackCursorPos;
    GLFWcursorenterfun      m_prevUserCallbackCursorEnter;
    GLFWmousebuttonfun      m_prevUserCallbackMousebutton;
    GLFWscrollfun           m_prevUserCallbackScroll;
    GLFWkeyfun              m_prevUserCallbackKey;
    GLFWcharfun             m_prevUserCallbackChar;
    GLFWmonitorfun          m_prevUserCallbackMonitor;
    
};
