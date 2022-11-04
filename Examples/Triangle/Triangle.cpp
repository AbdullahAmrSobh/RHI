#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <cassert>

#include <RHI/RHI.hpp>

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>

#define ASSERT_VALUE(exception) exception.or_else([](RHI::EResultCode resultCode) { assert(resultCode == RHI::EResultCode::Success); }).value_or(nullptr)

std::vector<uint32_t> ReadBinFile(std::string path)
{
    return std::vector<uint32_t> {};
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
            RHI::Win32SurfaceDesc surfaceDesc;
            surfaceDesc.hwnd;
            surfaceDesc.instance;
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

        // Create the frameGraph.
        {

        }
        
        // Create pipeline
        {
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
            pipelineDesc.pRenderPass                = &m_primaryRenderPass->GetPass();
            pipelineDesc.vertexInputAttributes.push_back(RHI::GraphicsPipelineVertexAttributeState{"v_position", RHI::EFormat::R32G32B32Sfloat});
            pipelineDesc.vertexInputAttributes.push_back(RHI::GraphicsPipelineVertexAttributeState{"v_color", RHI::EFormat::R8G8B8A8Srgb});
            
            m_pipeline = ASSERT_VALUE(m_device->CreateGraphicsPipelineState(pipelineDesc));
        }
        
        // Create shader resource group allocator
        {
            m_shaderResourceGroupAllocator = ASSERT_VALUE(m_device->CreateShaderResourceGroupAllocator());
        }
    };

    void OnFrame()
    {
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
    RHI::Unique<PrimaryRenderPass>                  m_primaryRenderPass;
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