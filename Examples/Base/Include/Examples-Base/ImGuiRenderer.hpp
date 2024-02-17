#pragma once

#include <imgui.h>

#include <RHI/RHI.hpp>

#include <GLFW/glfw3.h>

struct ImGuiRendererCreateInfo
{
    RHI::Context* context;
    RHI::FrameScheduler* scheduler;
    std::vector<uint8_t> shaderBlob;
    RHI::CommandListAllocator* commandAllocator;
};

class IMGUI_IMPL_API ImGuiRenderer
{
public:
    void Init(ImGuiRendererCreateInfo createInfo);
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

    // clang-format off
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
    // clang-format on
};
