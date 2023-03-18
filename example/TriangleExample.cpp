#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <RHI/RHI.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "stb_image.hpp"

std::vector<uint32_t> readBinFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    assert(file.is_open());

    auto                  fileSize = file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));

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
};

static std::vector<float>    vertexBufferData = {-0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,
                                                 0.5f,  0.5f,  0.0f, 0.0f, 1.0f, -0.5f, 0.5f,  1.0f, 1.0f, 1.0f};
static std::vector<uint32_t> indexBufferData  = {0, 1, 2, 2, 3, 0};

class Renderer
{
public:
    void Init()
    {
        // Create Instance and device.
        {
            m_instance = RHI::IInstance::Create(RHI::BackendType::Vulkan, std::make_unique<RHIDebugCallbacks>()).value();
            assert(!m_instance->GetPhysicalDevices().empty());
            const RHI::IPhysicalDevice* pPhysicalDevice = m_instance->GetPhysicalDevices().front();
            for (auto* device : m_instance->GetPhysicalDevices())
            {
                if (device->m_isDiscrete)
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
            m_window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
            if (!m_window)
            {
                glfwTerminate();
                std::exit(-1);
            }

            RHI::Win32SurfaceDesc surfaceDesc {};
            surfaceDesc.hwnd = glfwGetWin32Window(m_window);
            surfaceDesc.instance;

            m_surface = m_instance->CreateSurface(surfaceDesc).value();

            RHI::SwapchainDesc swapchainDesc;
            swapchainDesc.pSurface        = m_surface.get();
            swapchainDesc.backImagesCount = 2;
            swapchainDesc.extent          = {640, 480};
            swapchainDesc.pSurface        = m_surface.get();
            m_swapchain                   = m_device->CreateSwapChain(swapchainDesc).value();

            m_scissor.sizeX = swapchainDesc.extent.sizeX;
            m_scissor.sizeY = swapchainDesc.extent.sizeY;
            m_scissor.y     = 0;
            m_scissor.x     = 0;

            m_viewport.drawingArea.y     = 0;
            m_viewport.drawingArea.x     = 0;
            m_viewport.drawingArea.sizeX = swapchainDesc.extent.sizeX;
            m_viewport.drawingArea.sizeY = swapchainDesc.extent.sizeY;
            m_viewport.minDepth          = 0.0f;
            m_viewport.maxDepth          = 1.0f;
        }

        // Vertex Buffer Creation
        {
            RHI::AllocationDesc allocationDesc {};
            allocationDesc.type                            = RHI::MemoryLevel::Device;
            allocationDesc.byteOffset                      = 0;
            allocationDesc.memoryRequirement.byteSize      = vertexBufferData.size();
            allocationDesc.memoryRequirement.byteAlignment = alignof(uint32_t);
            allocationDesc.usage                           = RHI::MemoryUsage::Stage;

            RHI::BufferDesc bufferDesc {};
            bufferDesc.size  = vertexBufferData.size();
            bufferDesc.usage = RHI::BufferUsageFlagBits::Vertex;
            bufferDesc.usage = bufferDesc.usage | RHI::BufferUsageFlagBits::Transfer;

            m_vertexBuffer = m_device->CreateBuffer(allocationDesc, bufferDesc).value();

            assert(m_vertexBuffer->SetData<float>(0, vertexBufferData) == RHI::ResultCode::Success);

            allocationDesc.memoryRequirement.byteSize = indexBufferData.size();
            bufferDesc.size                           = indexBufferData.size() * sizeof(uint32_t);
            bufferDesc.usage                          = RHI::BufferUsageFlagBits::Index;
            bufferDesc.usage                          = bufferDesc.usage | RHI::BufferUsageFlagBits::Transfer;

            m_indexBuffer = m_device->CreateBuffer(allocationDesc, bufferDesc).value();

            assert(m_indexBuffer->SetData<uint32_t>(0, indexBufferData) == RHI::ResultCode::Success);
        }

        #if 0
        // Create shader resource group allocator
        {
            m_shaderResourceGroupAllocator = m_device->CreateShaderResourceGroupAllocator().value();
            RHI::ShaderInputResourceBindingDesc bindingDesc {};
            bindingDesc.access                                   = RHI::ShaderResourceAccessType::Read;
            bindingDesc.type                                     = RHI::ShaderInputResourceType::Image;
            bindingDesc.count                                    = 1;
            bindingDesc.name                                     = "triangleTexture";
            bindingDesc.stages                                   = RHI::ShaderStageFlagBits::Pixel;
            RHI::ShaderBindingReference imageSrgBindingReference = m_srgLayout.AddInputResource(bindingDesc);
            m_shaderResourceGroup                                = m_shaderResourceGroupAllocator->Allocate(m_srgLayout).value();

            RHI::ShaderResourceGroupData data {};
            std::vector<RHI::IImageView*> images {m_imageView.get()};
            data.BindImages(imageSrgBindingReference, images);

            m_shaderResourceGroup->Update(data);
        }
        #endif

        {
            m_scheduler = m_device->CreateFrameScheduler().value();

            const RHI::RenderPassProducerCallbacks::SetupAttachmentsCallback setupAttachmentsCallback = [&](RHI::FrameGraphBuilder& builder)
            {
                RHI::ImageViewDesc viewDesc {};
                viewDesc.format     = RHI::Format::B8G8R8A8_UNORM_SRGB;
                viewDesc.type       = RHI::ImageViewType::View2D;
                viewDesc.viewAspect = RHI::ImageViewAspectFlagBits::Color;
                builder.UseImageAttachment("colorAttachment", viewDesc);
            };

            const RHI::RenderPassProducerCallbacks::BuildCommandBufferCallback buildCommandBufferCallback =
                [&](RHI::ICommandBuffer& commandBuffer)
            {
                if (!m_pipeline)
                {
                    RHI::ShaderProgramDesc shaderDesc {};
                    shaderDesc.entryName  = "main";
                    shaderDesc.shaderCode = readBinFile("./shaders/triangle.vert.spv");
                    shaderDesc.stage      = RHI::ShaderStageFlagBits::Vertex;
                    auto vertexShader     = m_device->CreateShaderProgram(shaderDesc).value();
                    shaderDesc.shaderCode = readBinFile("./shaders/triangle.frag.spv");
                    shaderDesc.stage      = RHI::ShaderStageFlagBits::Pixel;
                    auto pixelShader      = m_device->CreateShaderProgram(shaderDesc).value();

                    RHI::GraphicsPipelineStateDesc desc {};
                    desc.pipelineLayoutDesc.shaderBindingGroupLayouts.push_back(m_srgLayout);
                    desc.shaderStages.pVertexShader = vertexShader.get();
                    desc.shaderStages.pPixelShader  = pixelShader.get();
                    desc.vertexInputAttributes.push_back(RHI::GraphicsPipelineVertexAttributeState {"position", RHI::Format::R32G32_FLOAT});
                    desc.vertexInputAttributes.push_back(RHI::GraphicsPipelineVertexAttributeState {"color", RHI::Format::R32G32B32_FLOAT});
                    desc.pRenderPass = m_producer->GetRenderPass();
                    m_pipeline       = m_device->CreateGraphicsPipelineState(desc).value();
                }

                commandBuffer.SetViewports(std::vector {m_viewport});
                commandBuffer.SetScissors(std::vector {m_scissor});

                RHI::DrawCommand::IndexedDrawData drawData;
                drawData.indexCount = RHI::CountElements(indexBufferData);
                RHI::DrawCommand drawCommand;
                drawCommand.drawData                 = drawData;
                drawCommand.pipelineState            = m_pipeline.get();
                drawCommand.vertexBuffer             = m_vertexBuffer.get();
                drawCommand.indexBuffer              = m_indexBuffer.get();
                drawCommand.shaderResourceGroup      = m_shaderResourceGroup.get();
                drawCommand.shaderResourceGroupIndex = 0;
                commandBuffer.Submit(drawCommand);
            };

            m_producer =
                std::make_unique<RHI::RenderPassProducerCallbacks>("mainRenderpass", setupAttachmentsCallback, buildCommandBufferCallback);
        }
    }

    void OnFrame()
    {
        m_scheduler->Begin();
        m_scheduler->GetAttachmentsRegistry().ImportSwapchain("colorAttachment", *m_swapchain);
        m_scheduler->Schedule(*m_producer);
        m_scheduler->End();
    }

public:
    GLFWwindow* m_window = nullptr;

    RHI::Rect     m_scissor  = {};
    RHI::Viewport m_viewport = {};

    std::unique_ptr<RHI::IInstance> m_instance;

    std::unique_ptr<RHI::IDevice> m_device;

    std::unique_ptr<RHI::ISurface> m_surface;

    std::unique_ptr<RHI::ISwapchain> m_swapchain;

    std::unique_ptr<RHI::IImage> m_imageResource;

    std::unique_ptr<RHI::IImageView> m_imageView;

    std::unique_ptr<RHI::IBuffer> m_vertexBuffer;

    std::unique_ptr<RHI::IBuffer> m_indexBuffer;

    RHI::ShaderResourceGroupLayout m_srgLayout {};

    std::unique_ptr<RHI::IShaderResourceGroupAllocator> m_shaderResourceGroupAllocator;

    std::unique_ptr<RHI::IShaderResourceGroup> m_shaderResourceGroup;

    std::unique_ptr<RHI::IPipelineState> m_pipeline;

    std::unique_ptr<RHI::IFrameScheduler> m_scheduler;

    std::unique_ptr<RHI::RenderPassProducerCallbacks> m_producer;
};

int main(void)
{
    Renderer renderer;
    if (!glfwInit())
        return -1;

    renderer.Init();

    while (!glfwWindowShouldClose(renderer.m_window))
    {
        renderer.OnFrame();
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}