#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <cassert>

#include <RHI/RHI.hpp>
#include "RHI/Commands.hpp"
#include "RHI/Common.hpp"
#include "RHI/Format.hpp"
#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/FrameGraphPass.hpp"
#include "RHI/Platform/XlibSurface.hpp"
#include "RHI/Resource.hpp"
#include "RHI/Swapchain.hpp"

#define ASSERT_VALUE(exception) exception.or_else([](RHI::EResultCode resultCode) { assert(resultCode == RHI::EResultCode::Success); }).value_or(nullptr)

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>

std::vector<uint32_t> ReadBinFile(std::string_view path)
{
    std::vector<uint32_t> buffer;
    return buffer;
}

struct Mesh
{
    std::vector<float>    vertexBufferData;
    std::vector<uint32_t> indexBufferData;
};

Mesh LoadMesh(std::string_view path)
{
    return Mesh();
}

class RHIDebugCallbacks final : public RHI::IDebugCallbacks
{
public:
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

        // Create shader resource group allocator
        {
            RHI::ShaderResourceGroupLayout layout;
            m_shaderResourceGroupAllocator = ASSERT_VALUE(m_device->CreateShaderResourceGroupAllocator());
            m_shaderResourceGroup          = ASSERT_VALUE(m_shaderResourceGroupAllocator->Allocate(layout));
        }

        // Create the frameGraph.
        {
            m_frameGraph = ASSERT_VALUE(m_device->CreateFrameGraph());
            // Import the swapchain as an imageAttachment.
            swapchainImageReference = m_frameGraph->GetAttachmentsRegistry().ImportSwapchain("SwapchainImage", *m_swapchain);
            RHI::ImageFrameAttachmentDesc depthStencilAttachmentDesc;
            depthStencilAttachmentDesc.name                     = "depthStencil";
            depthStencilAttachmentDesc.imageDesc.arraySize      = 1;
            depthStencilAttachmentDesc.imageDesc.sampleCount    = RHI::ESampleCount::Count1;
            depthStencilAttachmentDesc.imageDesc.format         = RHI::EFormat::D32Sfloat;
            depthStencilAttachmentDesc.imageDesc.mipLevelsCount = 1;
            depthStencilAttachmentDesc.imageDesc.usage          = RHI::EImageUsageFlagBits::DepthStencil;
            depthStencilAttachmentDesc.imageDesc.extent         = m_swapchain->GetBackBuffersDesc().extent;
            depthStencilAttachmentReference = m_frameGraph->GetAttachmentsRegistry().CreateTransientImageAttachment(depthStencilAttachmentDesc);

            m_mainRenderPass = ASSERT_VALUE(m_frameGraph->CreatePass("PrimaryRenderPass", RHI::EHardwareQueueType::Graphics));

            RHI::PassCallbacksFn::SetupCallback setupCallback = [&](RHI::FrameGraphBuilder& builder)
            {
                RHI::ImagePassAttachmentDesc attachmentDesc;
                attachmentDesc.attachmentReference           = swapchainImageReference;
                attachmentDesc.attachmentViewDesc.viewAspect = RHI::EImageViewAspectFlagBits::Color;
                attachmentDesc.attachmentViewDesc.format     = m_swapchain->GetBackBuffersDesc().format;
                attachmentDesc.attachmentViewDesc.range      = {0, 1, 0, 1};
                attachmentDesc.attachmentViewDesc.type       = RHI::EImageViewType::Type2D;
                builder.UseImageAttachment(attachmentDesc, RHI::EAttachmentUsage::RenderTarget, RHI::EAttachmentAccess::Write);

                RHI::ImagePassAttachmentDesc depthAttachmentDesc;
                depthAttachmentDesc.attachmentReference           = swapchainImageReference;
                depthAttachmentDesc.attachmentViewDesc.viewAspect = RHI::EImageViewAspectFlagBits::Depth;
                depthAttachmentDesc.attachmentViewDesc.format     = RHI::EFormat::D32Sfloat;
                depthAttachmentDesc.attachmentViewDesc.range      = {0, 1, 0, 1};
                depthAttachmentDesc.attachmentViewDesc.type       = RHI::EImageViewType::Type2D;
                builder.UseImageAttachment(depthAttachmentDesc, RHI::EAttachmentUsage::DepthStencil, RHI::EAttachmentAccess::Write);
            };

            RHI::PassCallbacksFn::CompileCallback compileCallback = [&](RHI::PassCompileContext& passCompileContext) {

            };

            RHI::PassCallbacksFn::ExecuteCallback executeCallback = [&](RHI::ICommandBuffer& commandBuffer)
            {
                commandBuffer.Begin();
                commandBuffer.SetScissors({m_scissor});
                commandBuffer.SetViewports({m_viewport});
                RHI::DrawCommand draw{*m_pipeline, RHI::DrawCommand::IndexedData()};
                draw.BindShaderResourceGroup("mainSrg", *m_shaderResourceGroup);
                commandBuffer.Submit(draw);
                commandBuffer.End();
            };

            auto passCallbacks = RHI::CreateUnique<RHI::PassCallbacksFn>(setupCallback, compileCallback, executeCallback);

            m_mainRenderPass->SetCallbacks(*passCallbacks);
            m_mainRenderPass->Compile();
        }

        // Create pipeline
        RHI::ShaderProgramDesc desc;
        desc.shaderCode   = ReadBinFile("./shaders.spv");
        desc.stage        = RHI::EShaderStageFlagBits::Vertex;
        desc.entryName    = "vertexShader";
        auto vertexShader = ASSERT_VALUE(m_device->CreateShaderProgram(desc));

        desc.shaderCode  = ReadBinFile("./shaders.spv");
        desc.stage       = RHI::EShaderStageFlagBits::Pixel;
        desc.entryName   = "pixelShader";
        auto pixelShader = ASSERT_VALUE(m_device->CreateShaderProgram(desc));

        RHI::GraphicsPipelineStateDesc pipelineDesc;
        pipelineDesc.shaderStages.pVertexShader = vertexShader.get();
        pipelineDesc.shaderStages.pPixelShader  = pixelShader.get();
        pipelineDesc.pPass                      = m_mainRenderPass.get();
        pipelineDesc.vertexInputAttributes.push_back(RHI::GraphicsPipelineVertexAttributeState{"v_position", RHI::EFormat::R32G32B32Sfloat});
        pipelineDesc.vertexInputAttributes.push_back(RHI::GraphicsPipelineVertexAttributeState{"v_color", RHI::EFormat::R8G8B8A8Srgb});

        m_pipeline = ASSERT_VALUE(m_device->CreateGraphicsPipelineState(pipelineDesc));
    };

    void FramePrepare() 
    {
    }

    void OnFrame()
    {
        m_frameGraph->BeginFrame();

        m_mainRenderPass->Submit();

        m_frameGraph->EndFrame();
    };

private:
    GLFWwindow* m_pWindow;  

    RHI::ImageAttachmentReference swapchainImageReference{0};
    RHI::ImageAttachmentReference depthStencilAttachmentReference{0};

    RHI::Rect     m_scissor;
    RHI::Viewport m_viewport;

    RHI::Unique<RHI::IInstance>                     m_instance;
    RHI::Unique<RHI::IDevice>                       m_device;
    RHI::Unique<RHI::ISurface>                      m_surface;
    RHI::Unique<RHI::ISwapchain>                    m_swapchain;
    RHI::Unique<RHI::IBuffer>                       m_vertexBuffer;
    RHI::Unique<RHI::IBuffer>                       m_indexBuffer;
    RHI::Unique<RHI::IShaderResourceGroupAllocator> m_shaderResourceGroupAllocator;

    RHI::Unique<RHI::IShaderResourceGroup> m_shaderResourceGroup;
    RHI::Unique<RHI::IPipelineState>       m_pipeline;
    RHI::Unique<RHI::IFrameGraph>          m_frameGraph;
    RHI::Unique<RHI::IPass>                m_mainRenderPass;
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
    glfwTerminate();
    return 0;
}