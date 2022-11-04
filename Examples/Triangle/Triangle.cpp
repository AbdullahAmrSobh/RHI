#include <cstdint>
#include <cstdio>
#include <fstream>
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

#include "RHI/Resource.hpp"
#include "RHI/Swapchain.hpp"

#ifdef RHI_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include "RHI/Platform/Win32Surface.hpp"
#elif defined(RHI_LINUX)
#define GLFW_EXPOSE_NATIVE_X11
#include "RHI/Platform/XlibSurface.hpp"
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

template <typename T>
inline constexpr auto GetExpectedValue(T&& t) -> typename T::value_type
{
    assert(t.has_value());
    return std::move(t.value());
}

std::vector<uint32_t> ReadBinFile(std::string path)
{
    return std::vector<uint32_t>{};
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

class PrimaryRenderPass final : public RHI::IPassCallbacks
{
public:
    PrimaryRenderPass(RHI::IFrameGraph& frameGraph)
        : RHI::IPassCallbacks(frameGraph, "primaryRenderPass", RHI::EPassType::Graphics)
    {
    }

    virtual ~PrimaryRenderPass() = default;

    virtual void Setup(RHI::FrameGraphBuilder& builder) override {}

    virtual void UseAttachments(RHI::IPass& pass) override {}

    virtual void BuildCommandBuffer(RHI::ICommandBuffer& commandBuffer) override {}
};

class Renderer
{
public:
    void Init()
    {
        // Create Instance and device.
        {
            m_instance = GetExpectedValue(RHI::IInstance::Create(RHI::EBackend::Vulkan, RHI::CreateUnique<RHIDebugCallbacks>()));

            RHI::IPhysicalDevice* pPhysicalDevice = m_instance->GetPhysicalDevices().front();

            m_device = GetExpectedValue(m_instance->CreateDevice(*pPhysicalDevice));
        }

        // Create GLFWWindow, Surface and Swapchain
        {
            /* Create a windowed mode window and its OpenGL context */
            m_pWindow = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
            if (!m_pWindow)
            {
                glfwTerminate();
                std::exit(-1);
            }
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

            RHI::Win32SurfaceDesc surfaceDesc;
            surfaceDesc.hwnd = glfwGetWin32Window(m_pWindow);
            surfaceDesc.instance;
            m_surface = GetExpectedValue(m_instance->CreateSurface(surfaceDesc));

            RHI::SwapchainDesc swapchainDesc;
            swapchainDesc.pSurface        = m_surface.get();
            swapchainDesc.backImagesCount = 2;
            swapchainDesc.extent          = {640, 480};
            swapchainDesc.pSurface        = m_surface.get();
            m_swapchain                   = GetExpectedValue(m_device->CreateSwapChain(swapchainDesc));
        }

        std::vector<float>    vertexBufferData = {0.0f, -0.5f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f};
        std::vector<uint32_t> indexBufferData  = {0, 1, 2};

        // Vertex Buffer Creation
        {
            RHI::AllocationDesc allocationDesc;
            allocationDesc.type                            = RHI::EMemoryType::Device;
            allocationDesc.byteOffset                      = 0;
            allocationDesc.pMemoryPool                     = nullptr;
            allocationDesc.memoryRequirement.byteSize      = vertexBufferData.size();
            allocationDesc.memoryRequirement.byteAlignment = alignof(uint32_t);
            allocationDesc.usage                           = RHI::EMemoryUsage::Stage;

            RHI::BufferDesc bufferDesc;
            bufferDesc.size  = vertexBufferData.size();
            bufferDesc.usage = RHI::EBufferUsageFlagBits::Vertex;
            bufferDesc.usage = bufferDesc.usage | RHI::EBufferUsageFlagBits::Transfer;
            
            m_vertexBuffer = GetExpectedValue(m_device->CreateBuffer(allocationDesc, bufferDesc));

            allocationDesc.memoryRequirement.byteSize = indexBufferData.size();
            bufferDesc.size                           = indexBufferData.size();
            bufferDesc.usage                          = RHI::EBufferUsageFlagBits::Index;
            bufferDesc.usage                          = bufferDesc.usage | RHI::EBufferUsageFlagBits::Transfer;

            m_indexBuffer = GetExpectedValue(m_device->CreateBuffer(allocationDesc, bufferDesc));
        }

        // Create the frameGraph.
        {
        
        }

        // Create pipeline
        {
            RHI::ShaderProgramDesc desc;

            desc.shaderCode   = ReadBinFile("./shaders.spv");
            desc.stage        = RHI::EShaderStageFlagBits::Vertex;
            desc.entryName    = "vertexShader";
            auto vertexShader = GetExpectedValue(m_device->CreateShaderProgram(desc));

            desc.shaderCode  = ReadBinFile("./shaders.spv");
            desc.stage       = RHI::EShaderStageFlagBits::Pixel;
            desc.entryName   = "pixelShader";
            auto pixelShader = GetExpectedValue(m_device->CreateShaderProgram(desc));

            RHI::GraphicsPipelineStateDesc pipelineDesc;
            pipelineDesc.shaderStages.pVertexShader = vertexShader.get();
            pipelineDesc.shaderStages.pPixelShader  = pixelShader.get();
            pipelineDesc.pRenderPass                = &m_primaryRenderPass->GetPass();
            pipelineDesc.vertexInputAttributes.push_back(RHI::GraphicsPipelineVertexAttributeState{"v_position", RHI::EFormat::R32G32B32Sfloat});
            pipelineDesc.vertexInputAttributes.push_back(RHI::GraphicsPipelineVertexAttributeState{"v_color", RHI::EFormat::R8G8B8A8Srgb});

            m_pipeline = GetExpectedValue(m_device->CreateGraphicsPipelineState(pipelineDesc));
        }

        // Create shader resource group allocator
        {
            m_shaderResourceGroupAllocator = GetExpectedValue(m_device->CreateShaderResourceGroupAllocator());
        }
    };
    
    void OnFrame(){};

public:
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
    RHI::Unique<RHI::IPipelineState>                m_pipeline;
    RHI::Unique<RHI::IFrameGraph>                   m_frameGraph;
    RHI::Unique<PrimaryRenderPass>                  m_primaryRenderPass;
};

int main(void)
{
    Renderer    renderer;
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