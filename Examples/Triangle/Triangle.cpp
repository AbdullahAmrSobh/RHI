#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <cassert>

#include <RHI/RHI.hpp>

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>

#define ASSERT_VALUE(exception) exception.or_else([](RHI::EResultCode resultCode) { assert(resultCode == RHI::EResultCode::Success); }).value_or(nullptr)

std::vector<std::byte> ReadBinFile(std::string filepath)
{
    std::vector<std::byte> buffer;
    return buffer;
}

class RHIDebugCallbacks final : public RHI::IDebugCallbacks
{
public:
    RHIDebugCallbacks()                         = default;
    RHIDebugCallbacks(RHIDebugCallbacks&&)      = delete;
    RHIDebugCallbacks(const RHIDebugCallbacks&) = delete;
    ~RHIDebugCallbacks()                        = default;

    virtual void Log(std::string_view message) override
    {
        std::cout << "Debug messenger: " << message << std::endl;
    }

    virtual void Warn(std::string_view message) override
    {
        std::cout << "Debug messenger: " << message << std::endl;
    }

    virtual void Error(std::string_view message) override
    {
        std::cout << "Debug messenger: " << message << std::endl;
    }

    virtual void Fatel(std::string_view message) override
    {
        std::cout << "Debug messenger: " << message << std::endl;
    }
};

class PrimaryRenderPass final : public RHI::Pass::Callbacks
{
public:
    virtual void Setup(RHI::FrameGraph::Builder& frameGraph) override
    {
        RHI::ImagePassAttachmentDesc colorAttachment;
        colorAttachment.id                          = "a_swapchainImage";
        colorAttachment.view.viewAspect             = RHI::EImageViewAspectFlagBits::Color;
        colorAttachment.view.format                 = RHI::EFormat::Undefined;
        colorAttachment.view.type                   = RHI::EImageType::Type2D;
        colorAttachment.view.range.arraySize        = 1;
        colorAttachment.view.range.baseArrayElement = 0;
        colorAttachment.view.range.mipLevelsCount   = 1;
        colorAttachment.view.range.baseMipLevel     = 0;

        frameGraph.UseColorAttachment(colorAttachment, RHI::EAccess::ReadWrite);

        RHI::ImagePassAttachmentDesc depthAttachment;
        frameGraph.UseColorAttachment(depthAttachment, RHI::EAccess::ReadWrite);
    }

    virtual void BindAttachments(RHI::AttachmentsBindingContext& context) override
    {
        RHI::IShaderResourceGroup::Data data;
        auto                            pImageView = context.GetImageView("a_swapchainImage");
        data.BindImage(RHI::ShaderBindingIndex(0), *pImageView);
        m_pShaderResourceGroup->BindData(data);
    }

    virtual void Execute(RHI::ExecuteContext& context) override
    {
        auto& commandBuffer = context.GetCommandBuffer();

        commandBuffer.Begin();
        std::vector<RHI::Viewport> viewports = {};
        std::vector<RHI::Rect>     scissors  = {};
        commandBuffer.SetViewports(viewports);
        commandBuffer.SetScissors(scissors);
        RHI::DrawCommand drawCommand{
            *m_pPipelineState,
            RHI::DrawCommand::IndexedData{},
        };
        commandBuffer.Submit(drawCommand);
        commandBuffer.End();
    }

    RHI::IPipelineState*       m_pPipelineState;
    RHI::IShaderResourceGroup* m_pShaderResourceGroup;
};

class Renderer
{
public:
    void Init()
    {
        // Create Instance and device.
        {
            RHI::Unique<RHIDebugCallbacks> debugCallbacks;
            m_instance = ASSERT_VALUE(RHI::IInstance::Create(RHI::EBackend::Vulkan, std::move(debugCallbacks)));

            RHI::IPhysicalDevice* pPhysicalDevice;
            for (auto& physicalDevice : m_instance->GetPhysicalDevice())
            {
                pPhysicalDevice = physicalDevice;
            }

            m_device = ASSERT_VALUE(m_instance->CreateDevice(*pPhysicalDevice));
        }

        // Create Surface and Swapchain
        {
            RHI::X11SurfaceDesc        surfaceDesc;
            RHI::Unique<RHI::ISurface> surface = ASSERT_VALUE(m_instance->CreateSurface(surfaceDesc));

            RHI::SwapchainDesc swapchainDesc;
            swapchainDesc.backImagesCount          = 2;
            swapchainDesc.extent                   = {640, 480};
            swapchainDesc.pSurface                 = m_surface.get();
            RHI::Unique<RHI::ISwapchain> swapchain = ASSERT_VALUE(m_device->CreateSwapChain(swapchainDesc));
        }

        std::vector<float>    vertexBufferData;
        std::vector<uint32_t> indexBufferData;

        // Vertex Buffer Creation
        {
            RHI::AllocationDesc allocationDesc;
            allocationDesc.type                            = RHI::EMemoryType::Device;
            allocationDesc.byteOffset                      = 0;
            allocationDesc.pMemoryPool                     = nullptr;
            allocationDesc.memoryRequirement.byteSize      = vertexBufferData.size();
            allocationDesc.memoryRequirement.byteAlignment = alignof(uint32_t);

            RHI::BufferDesc bufferDesc;
            bufferDesc.size  = vertexBufferData.size();
            bufferDesc.usage = RHI::EBufferUsageFlagBits::Vertex;
            bufferDesc.usage = bufferDesc.usage | RHI::EBufferUsageFlagBits::Transfer;

            m_vertexBuffer = ASSERT_VALUE(m_device->CreateBuffer(allocationDesc, bufferDesc));

            allocationDesc.memoryRequirement.byteSize = indexBufferData.size();
            bufferDesc.size                           = indexBufferData.size();
            bufferDesc.usage                          = RHI::EBufferUsageFlagBits::Index;
            bufferDesc.usage                          = bufferDesc.usage | RHI::EBufferUsageFlagBits::Transfer;

            m_indexBuffer = ASSERT_VALUE(m_device->CreateBuffer(allocationDesc, bufferDesc));
        }

        RHI::RenderTargetLayout renderTargetLayout;

        // Create pipeline
        {
            RHI::ShaderProgramDesc desc;

            desc.binShader    = ReadBinFile("./shaders.spv");
            desc.stage        = RHI::EShaderStageFlagBits::Vertex;
            desc.entryName    = "vertexShader";
            auto vertexShader = ASSERT_VALUE(m_device->CreateShaderProgram(desc));

            desc.binShader   = ReadBinFile("./shaders.spv");
            desc.stage       = RHI::EShaderStageFlagBits::Pixel;
            desc.entryName   = "pixelShader";
            auto pixelShader = ASSERT_VALUE(m_device->CreateShaderProgram(desc));
            
            RHI::GraphicsPipelineStateDesc pipelineDesc;
            pipelineDesc.shaderStages.pVertexShader = vertexShader.get();
            pipelineDesc.shaderStages.pPixelShader  = pixelShader.get();
            pipelineDesc.renderTargetLayout         = renderTargetLayout;
            pipelineDesc.vertexInputAttributes.push_back(RHI::PipelineStateDesc::VertexAttribute{"v_position", RHI::EFormat::R32G32B32Sfloat});
            pipelineDesc.vertexInputAttributes.push_back(RHI::PipelineStateDesc::VertexAttribute{"v_color", RHI::EFormat::R8G8B8A8Srgb});

            m_pipeline = ASSERT_VALUE(m_device->CreateGraphicsPipelineState(pipelineDesc));
        }

        // Create shader resource group allocator
        {
            m_shaderResourceGroupAllocator = ASSERT_VALUE(m_device->CreateShaderResourceGroupAllocator());
        }

        {
            m_frameGraph   = ASSERT_VALUE(m_device->CreateFrameGraph());
            auto& database = m_frameGraph->GetDatabase();
            database.UseSwapchainImage("a_swapchainImage)", *m_swapchain);
            
            m_pPrimaryPass = &m_frameGraph->AddRenderPass("PrimaryRenderPass", RHI::EHardwareQueueType::Graphics);
            m_pPrimaryPass->SetCallbacks(m_primaryPassCallbacks);
        }
    };

    void Shutdown(){

    };

    void OnFrame()
    {
        m_frameGraph->BeginFrame();
        m_pPrimaryPass->Submit();
        m_frameGraph->EndFrame();
    };

private:
    GLFWwindow* m_pWindow;

    RHI::Unique<RHI::IInstance>                     m_instance;
    RHI::Unique<RHI::IDevice>                       m_device;
    RHI::Unique<RHI::ISurface>                      m_surface;
    RHI::Unique<RHI::ISwapchain>                    m_swapchain;
    RHI::Unique<RHI::IBuffer>                       m_vertexBuffer;
    RHI::Unique<RHI::IBuffer>                       m_indexBuffer;
    RHI::Unique<RHI::IPipelineState>                m_pipeline;
    RHI::Unique<RHI::IShaderResourceGroupAllocator> m_shaderResourceGroupAllocator;
    RHI::Unique<RHI::IFrameGraph>                   m_frameGraph;
    RHI::Unique<RHI::IPass>                         m_pPrimaryPass;
    PrimaryRenderPass                               m_primaryPassCallbacks;
};

int main(void)
{
    GLFWwindow* window;
    Renderer    renderer;
    /* Initialize the library */
    if (!glfwInit())
        return -1;
    renderer.Init();

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        renderer.OnFrame();
        /* Poll for and process events */
        glfwPollEvents();
    }
    renderer.Shutdown();
    glfwTerminate();
    return 0;
}