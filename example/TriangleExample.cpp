#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <RHI/RHI.hpp>

#ifdef RHI_WINDOWS
#    define GLFW_EXPOSE_NATIVE_WIN32
#    include "RHI/Platform/Win32Surface.hpp"
#elif defined(RHI_LINUX)
#    define GLFW_EXPOSE_NATIVE_X11
#    include "RHI/Platform/XlibSurface.hpp"
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

std::vector<uint32_t> ReadBinFile(std::string filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    assert(file.is_open());

    auto fileSize = file.tellg();

    assert(fileSize % 4 == 0);

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()),
              static_cast<std::streamsize>(fileSize));

    file.close();

    return buffer;
}

class RHIDebugCallbacks final : public RHI::IDebugCallbacks
{
public:
    void Log(std::string_view message) override
    {
        std::cout << "Debug messenger: " << message << std::endl;
    }

    void Warn(std::string_view message) override
    {
        std::cout << "Debug messenger: " << message << std::endl;
    }

    void Error(std::string_view message) override
    {
        std::cout << "Debug messenger: " << message << std::endl;
    }

    void Fatel(std::string_view message) override
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
            m_instance =
                RHI::IInstance::Create(RHI::EBackend::Vulkan,
                                       RHI::CreateUnique<RHIDebugCallbacks>())
                    .value();
            assert(!m_instance->GetPhysicalDevices().empty());
            const RHI::IPhysicalDevice* pPhysicalDevice =
                m_instance->GetPhysicalDevices().front();
            for (auto* device : m_instance->GetPhysicalDevices())
            {
                if (device->IsDiscrete())
                {
                    pPhysicalDevice = device;
                }
            }
            m_device = m_instance->CreateDevice(*pPhysicalDevice).value();
        }

        // Create GLFWWindow, Surface and Swapchain
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            /* Create a windowed mode window and its OpenGL context */
            m_pWindow = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
            if (!m_pWindow)
            {
                glfwTerminate();
                std::exit(-1);
            }

#ifdef RHI_WINDOWS
            RHI::Win32SurfaceDesc surfaceDesc {};
            surfaceDesc.hwnd = glfwGetWin32Window(m_pWindow);
            surfaceDesc.instance;
#elif defined(RHI_LINUX)
            RHI::X11SurfaceDesc surfaceDesc {};
            surfaceDesc.window   = glfwGetX11Window(m_pWindow);
            surfaceDesc.pDisplay = glfwGetX11Display();
#endif

            m_surface = m_instance->CreateSurface(surfaceDesc).value();

            RHI::SwapchainDesc swapchainDesc;
            swapchainDesc.pSurface        = m_surface.get();
            swapchainDesc.backImagesCount = 2;
            swapchainDesc.extent          = {640, 480};
            swapchainDesc.pSurface        = m_surface.get();
            m_swapchain = m_device->CreateSwapChain(swapchainDesc).value();
        }

        std::vector<float>    vertexBufferData = {0.0f,
                                                  -0.5f,
                                                  1.0f,
                                                  0.0f,
                                                  0.0f,
                                                  0.5f,
                                                  0.5f,
                                                  0.0f,
                                                  1.0f,
                                                  0.0f,
                                                  -0.5f,
                                                  0.5f,
                                                  0.0f,
                                                  0.0f,
                                                  1.0f};
        std::vector<uint32_t> indexBufferData  = {0, 1, 2};

        // Vertex Buffer Creation
        {
            RHI::AllocationDesc allocationDesc {};
            allocationDesc.type        = RHI::EMemoryType::Device;
            allocationDesc.byteOffset  = 0;
            allocationDesc.pMemoryPool = nullptr;
            allocationDesc.memoryRequirement.byteSize = vertexBufferData.size();
            allocationDesc.memoryRequirement.byteAlignment = alignof(uint32_t);
            allocationDesc.usage = RHI::EMemoryUsage::Stage;

            RHI::BufferDesc bufferDesc {};
            bufferDesc.size  = vertexBufferData.size();
            bufferDesc.usage = RHI::EBufferUsageFlagBits::Vertex;
            bufferDesc.usage =
                bufferDesc.usage | RHI::EBufferUsageFlagBits::Transfer;

            m_vertexBuffer =
                m_device->CreateBuffer(allocationDesc, bufferDesc).value();

            allocationDesc.memoryRequirement.byteSize = indexBufferData.size();
            bufferDesc.size                           = indexBufferData.size();
            bufferDesc.usage = RHI::EBufferUsageFlagBits::Index;
            bufferDesc.usage =
                bufferDesc.usage | RHI::EBufferUsageFlagBits::Transfer;

            m_indexBuffer =
                m_device->CreateBuffer(allocationDesc, bufferDesc).value();
        }

        // Create shader resource group allocator
        {
            m_shaderResourceGroupAllocator =
                m_device->CreateShaderResourceGroupAllocator().value();
            RHI::ShaderInputResourceBindingDesc bindingDesc {};
            bindingDesc.access = RHI::EAccess::Read;
            bindingDesc.type   = RHI::EShaderInputResourceType::Image;
            bindingDesc.count  = 1;
            bindingDesc.name   = "triangleTexture";
            bindingDesc.stages = RHI::EShaderStageFlagBits::Pixel;
            RHI::ShaderResourceGroupLayout srgLayout {};
            srgLayout.AddInputResource(bindingDesc);
            m_shaderResourceGroup =
                m_shaderResourceGroupAllocator->Allocate(srgLayout).value();
        }

        m_scheduler = m_device->CreateFrameScheduler().value();
        m_scheduler->GetAttachmentsRegistry().ImportSwapchain(
            "swapchainImage", std::move(m_swapchain));

        RHI::RenderPassCallbacks::SetupAttachmentsCallback attachmentSetup =
            [&](RHI::FrameGraphBuilder& builder)
        {
            std::cout << "Setting up the framegraph. " << std::endl;
            RHI::ImagePassAttachmentDesc desc {};
            desc.name            = "swapchainImage";
            desc.desc.format     = RHI::EFormat::B8G8R8Srgb;
            desc.desc.type       = RHI::EImageViewType::Type2D;
            desc.desc.viewAspect = RHI::EImageViewAspectFlagBits::Color;
            desc.desc.range      = RHI::ImageViewRange();
            builder.UseImageAttachment(desc,
                                       RHI::AttachmentUsage::RenderTarget,
                                       RHI::AttachmentAccess::Write);
        };

        RHI::RenderPassCallbacks::BuildCommandListCallback buildCommandList =
            [&](RHI::ICommandBuffer& commandBuffer)
        {
            commandBuffer.Begin();
            commandBuffer.SetScissors({m_scissor});
            commandBuffer.SetViewports({m_viewport});

            RHI::DrawCommand drawCommand {};
            drawCommand.pPipelineState = m_pipeline.get();
            drawCommand.pVertexBuffer  = m_vertexBuffer.get();
            drawCommand.pIndexBuffer   = m_indexBuffer.get();
            drawCommand.type           = RHI::DrawCommand::EType::Indexed;
            RHI::DrawCommand::IndexedDrawData drawData {};
            drawData.indexCount     = RHI::CountElements(indexBufferData);
            drawCommand.indexedData = drawData;
            commandBuffer.Submit(drawCommand);

            commandBuffer.End();
        };

        auto producer = RHI::CreateUnique<RHI::RenderPassCallbacks>(
            attachmentSetup, buildCommandList);

        m_renderpass = m_device
                           ->CreateRenderPass("primaryRenderPass",
                                              RHI::DeviceQueueType::Graphics)
                           .value();

        m_renderpass->SetCallbacks(std::move(producer));

        m_scheduler->BuildGraph(*m_renderpass);

        {
            RHI::ShaderProgramDesc shaderDesc {};
            shaderDesc.entryName  = "main";
            shaderDesc.shaderCode = ReadBinFile("./shaders/triangle.vert.spv");
            shaderDesc.stage      = RHI::EShaderStageFlagBits::Vertex;
            auto vertexShader =
                m_device->CreateShaderProgram(shaderDesc).value();
            shaderDesc.shaderCode = ReadBinFile("./shaders/triangle.frag.spv");
            shaderDesc.stage      = RHI::EShaderStageFlagBits::Pixel;
            auto pixelShader =
                m_device->CreateShaderProgram(shaderDesc).value();
            RHI::GraphicsPipelineStateDesc desc {};
            desc.pRenderPass                = m_renderpass.get();
            desc.shaderStages.pVertexShader = vertexShader.get();
            desc.shaderStages.pPixelShader  = pixelShader.get();
            desc.vertexInputAttributes.push_back(
                {"position", RHI::EFormat::R32G32Sfloat});
            desc.vertexInputAttributes.push_back(
                {"color", RHI::EFormat::R32G32B32Sfloat});
            m_pipeline = m_device->CreateGraphicsPipelineState(desc).value();
        }
    };

    void OnFrame()
    {
        m_scheduler->BeginFrame();
        m_scheduler->Submit(*m_renderpass);
        m_scheduler->EndFrame();
    };

public:
    GLFWwindow* m_pWindow;

    RHI::Rect     m_scissor;
    RHI::Viewport m_viewport;

    RHI::Unique<RHI::IInstance>  m_instance;
    RHI::Unique<RHI::IDevice>    m_device;
    RHI::Unique<RHI::ISurface>   m_surface;
    RHI::Unique<RHI::ISwapchain> m_swapchain;
    RHI::Unique<RHI::IBuffer>    m_vertexBuffer;
    RHI::Unique<RHI::IBuffer>    m_indexBuffer;
    RHI::Unique<RHI::IShaderResourceGroupAllocator>
                                           m_shaderResourceGroupAllocator;
    RHI::Unique<RHI::IShaderResourceGroup> m_shaderResourceGroup;
    RHI::Unique<RHI::IPipelineState>       m_pipeline;

    RHI::Unique<RHI::IFrameScheduler> m_scheduler;

    RHI::Unique<RHI::IRenderPass> m_renderpass;
};

int main(void)
{
    Renderer renderer;
    if (!glfwInit())
        return -1;

    renderer.Init();

    while (!glfwWindowShouldClose(renderer.m_pWindow))
    {
        renderer.OnFrame();
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}