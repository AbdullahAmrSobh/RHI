// Fix swapchain resizing and moving into other monitor
// Fix validation errors and change API if needed!
// Fix all memory and resource leaks!

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <Examples-Base/ApplicationBase.hpp>

#include <RHI/RHI.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include <tracy/Tracy.hpp>

#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Allocator/MemPlumber.hpp>
#include <TL/Defer.hpp>

#include "Camera.hpp"

using namespace Examples;

TL::Vector<uint8_t> CreateCheckerboardImage(RHI::ImageSize2D size, uint32_t squareSize)
{
    TL::Vector<uint8_t> image(size.width * size.height, 0);
    for (int y = 0; y < size.height; ++y)
    {
        for (int x = 0; x < size.width; ++x)
        {
            bool isWhiteSquare        = ((x / squareSize) % 2 == (y / squareSize) % 2);
            image[y * size.width + x] = isWhiteSquare ? 255 : 0;
        }
    }
    return image;
}

class Playground final : public ApplicationBase
{
public:
    Playground()
        : ApplicationBase("Playground", 1600, 900)
    {
    }

    RHI::Device*    m_device;
    RHI::Swapchain* m_swapchain;
    RHI::Queue*     m_queue;

    struct PerFrame
    {
        uint64_t                    m_timelineValue;
        RHI::Handle<RHI::BindGroup> m_bindGroup;
    };

    uint32_t m_frameIndex      = 0;
    PerFrame m_perFrameData[2] = {};

    RHI::Handle<RHI::PipelineLayout>   m_pipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline> m_graphicsPipeline;

    RHI::RenderGraph*         m_renderGraph;
    RHI::Handle<RHI::Pass>    m_mainPass;
    RHI::Handle<RHI::RGImage> m_colorAttachment;

    RHI::Handle<RHI::Image>  m_texture;
    RHI::Handle<RHI::Buffer> m_vertexBuffer;
    RHI::Handle<RHI::Buffer> m_indexBuffer;
    RHI::Handle<RHI::Buffer> m_uniformBuffer;

    Camera m_camera;

    PerFrame& AdvanceFrame()
    {
        m_frameIndex = (m_frameIndex + 1) % 2;
        return m_perFrameData[m_frameIndex];
    }

    PerFrame& GetCurrentFrame() { return m_perFrameData[m_frameIndex]; }

    void InitContextAndSwapchain()
    {
        RHI::ApplicationInfo appInfo{
            .applicationName    = "Example",
            .applicationVersion = {0, 1, 0},
            .engineName         = "Forge",
            .engineVersion      = {0, 1, 0},
        };
        m_device = RHI::CreateVulkanDevice(appInfo);

        auto [width, height] = m_window->GetWindowSize();
        RHI::SwapchainCreateInfo swapchainInfo{
            .name          = "Swapchain",
            .imageSize     = {width, height},
            .imageUsage    = RHI::ImageUsage::Color,
            .imageFormat   = RHI::Format::RGBA8_UNORM,
            .minImageCount = 3,
            .presentMode   = RHI::SwapchainPresentMode::Fifo,
            .win32Window   = {m_window->GetNativeHandle()},
        };

        m_swapchain = m_device->CreateSwapchain(swapchainInfo).release();

        m_queue = m_device->GetQueue(RHI::QueueType::Graphics);
    }

    void ShutdownContextAndSwapchain()
    {
        for (auto& frame : m_perFrameData)
        {
        }

        delete m_swapchain;

        RHI::DestroyVulkanDevice(m_device);
    }

    void InitPipelineAndLayout()
    {
        TL_defer
        {
            TL_LOG_INFO("Hello, world!");
        };

        TL::Vector<uint32_t> spv;
        auto                 spvBlock = TL::ReadBinaryFile("./Shaders/Basic.spv");
        spv.resize(spvBlock.size / 4);
        memcpy(spv.data(), spvBlock.ptr, spvBlock.size);
        auto shaderModule = m_device->CreateShaderModule(spv);
        TL::Allocator::Release(spvBlock, alignof(char));

        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{
            .name = "BGL-ViewUB",
            .bindings{
                {
                    .type   = RHI::BindingType::UniformBuffer,
                    .stages = RHI::ShaderStage::Pixel | RHI::ShaderStage::Vertex,
                },
                {
                    .type   = RHI::BindingType::SampledImage,
                    .stages = RHI::ShaderStage::Pixel,
                },
            },
        };
        auto bindGroupLayout = m_device->CreateBindGroupLayout(bindGroupLayoutCI);

        RHI::PipelineLayoutCreateInfo layoutCI{.name = "graphics-pipeline-layout", .layouts = {bindGroupLayout}};
        m_pipelineLayout = m_device->CreatePipelineLayout(layoutCI);

        RHI::GraphicsPipelineCreateInfo pipelineCI{
            .name               = "Hello-Triangle",
            .vertexShaderName   = "VSMain",
            .vertexShaderModule = shaderModule.get(),
            .pixelShaderName    = "PSMain",
            .pixelShaderModule  = shaderModule.get(),
            .layout             = m_pipelineLayout,
            .vertexBufferBindings{
                {
                    .binding    = 0,
                    .stride     = sizeof(glm::vec3),
                    .attributes = {{.format = RHI::Format::RGB32_FLOAT}},
                },
                {
                    .binding    = 1,
                    .stride     = sizeof(glm::vec3),
                    .attributes = {{.format = RHI::Format::RGBA32_FLOAT}},
                },
            },
            .renderTargetLayout{
                .colorAttachmentsFormats = RHI::Format::RGBA8_UNORM,
            },
            .colorBlendState{.blendStates = {{.blendEnable = true}}},
            .rasterizationState{.cullMode = RHI::PipelineRasterizerStateCullMode::None},
            .depthStencilState{
                .depthTestEnable  = false,
                .depthWriteEnable = false,
            },
        };
        m_graphicsPipeline = m_device->CreateGraphicsPipeline(pipelineCI);

        // init and update bind groups

        for (auto& frame : m_perFrameData)
        {
            frame.m_bindGroup = m_device->CreateBindGroup(bindGroupLayout);
            RHI::BindGroupUpdateInfo bindGroupUpdateInfo{
                .images{
                    {
                        .dstBinding      = 1,
                        .dstArrayElement = 0,
                        .images          = m_texture,
                    },
                },
                .buffers{
                    {
                        .dstBinding      = 0,
                        .dstArrayElement = 0,
                        .buffers         = m_uniformBuffer,
                        .subregions      = {},
                    },
                },
            };
            m_device->UpdateBindGroup(frame.m_bindGroup, bindGroupUpdateInfo);
        }

        m_device->DestroyBindGroupLayout(bindGroupLayout);
    }

    void ShutdownPipelineAndLayout()
    {
        for (auto& frame : m_perFrameData)
        {
            m_device->DestroyBindGroup(frame.m_bindGroup);
        }
        m_device->DestroyGraphicsPipeline(m_graphicsPipeline);
        m_device->DestroyPipelineLayout(m_pipelineLayout);
    }

    void InitRenderGraph()
    {
        auto [width, height] = m_window->GetWindowSize();
        m_renderGraph        = m_device->CreateRenderGraph().release();

        RHI::PassCreateInfo passCI{
            .name  = "main-pass",
            .flags = RHI::PassFlags::Graphics,
        };
        m_mainPass = m_renderGraph->CreatePass(passCI);

        m_colorAttachment = m_renderGraph->ImportSwapchain("main-output", *m_swapchain);

        m_renderGraph->PassUseImage(
            m_mainPass, m_colorAttachment, RHI::ImageUsage::Color, RHI::PipelineStage::ColorAttachmentOutput, RHI::Access::None);

        m_renderGraph->PassResize(m_mainPass, {width, height});
    }

    void ShutdownRenderGraph()
    {
        if (m_renderGraph) delete m_renderGraph;
    }

    void InitBuffers()
    {
        auto checkerboard = CreateCheckerboardImage({512, 512}, 32);
        m_texture         = RHI::Utils::CreateImageWithContent(
                        *m_device,
                        {
                                    .usageFlags = RHI::ImageUsage::CopyDst | RHI::ImageUsage::ShaderResource,
                                    .type       = RHI::ImageType::Image2D,
                                    .size       = {512, 512},
                                    .format     = RHI::Format::R8_UNORM,
                        },
                        TL::Block::Create(checkerboard))
                        .GetValue();

        // clang-format off
        glm::vec3 positionData[] = {
            {-1.0f,  1.0f, 1.0f},
            { 1.0f,  1.0f, 1.0f},
            { 1.0f, -1.0f, 1.0f},
            {-1.0f, -1.0f, 1.0f},
        };

        glm::vec4 colorData[] = {
            {1.0f, 0.0f, 0.5f, 1.0f},
            {0.0f, 1.0f, 0.5f, 1.0f},
            {0.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 1.0f, 1.0f},
        };

        uint16_t indexData[] = {
            0, 1, 2,
            0, 2, 3,
         };
        // clang-format on

        size_t vertexBufferSize = sizeof(glm::vec3) * 4 + sizeof(glm::vec4) * 4;
        size_t indexBufferSize  = sizeof(uint16_t) * 6;

        RHI::BufferCreateInfo vertexBufferCI{
            .name       = "vertex-buffer",
            .heapType   = RHI::MemoryType::GPUShared,
            .usageFlags = RHI::BufferUsage::Vertex,
            .byteSize   = vertexBufferSize,
        };
        m_vertexBuffer = m_device->CreateBuffer(vertexBufferCI).GetValue();

        auto vertexBufferPtr = m_device->MapBuffer(m_vertexBuffer);
        memcpy(vertexBufferPtr, positionData, sizeof(glm::vec3) * 4);
        memcpy((char*)vertexBufferPtr + sizeof(glm::vec3) * 4, colorData, sizeof(glm::vec4) * 4);
        m_device->UnmapBuffer(m_vertexBuffer);

        RHI::BufferCreateInfo indexBufferCI{
            .name       = "index-buffer",
            .heapType   = RHI::MemoryType::GPUShared,
            .usageFlags = RHI::BufferUsage::Index,
            .byteSize   = indexBufferSize,
        };
        m_indexBuffer = m_device->CreateBuffer(indexBufferCI).GetValue();

        auto indexBufferPtr = m_device->MapBuffer(m_indexBuffer);
        memcpy(indexBufferPtr, indexData, sizeof(uint16_t) * 6);
        m_device->UnmapBuffer(m_indexBuffer);

        // uniform buffer data
        struct UniformData
        {
            glm::mat4 translate;
        };

        UniformData           uniformData{.translate = glm::mat4(1.f)};
        RHI::BufferCreateInfo uniformBufferCI{
            .name       = "uniform-buffer",
            .heapType   = RHI::MemoryType::GPUShared,
            .usageFlags = RHI::BufferUsage::Uniform,
            .byteSize   = sizeof(UniformData),
        };
        m_uniformBuffer       = m_device->CreateBuffer(uniformBufferCI).GetValue();
        auto uniformBufferPtr = m_device->MapBuffer(m_uniformBuffer);
        memcpy(uniformBufferPtr, &uniformData, sizeof(UniformData));
        m_device->UnmapBuffer(m_uniformBuffer);
    }

    void ShutdownBuffers()
    {
        if (m_vertexBuffer) m_device->DestroyBuffer(m_vertexBuffer);
        if (m_indexBuffer) m_device->DestroyBuffer(m_indexBuffer);
        if (m_uniformBuffer) m_device->DestroyBuffer(m_uniformBuffer);
        if (m_texture) m_device->DestroyImage(m_texture);
    }

    void OnInit() override
    {
        InitContextAndSwapchain();
        InitBuffers();
        InitPipelineAndLayout();
        InitRenderGraph();

        auto [width, height] = m_window->GetWindowSize();
        m_camera.SetPerspective(30.0f, (float)width / (float)height, 0.00001f, 100000.0f);
        m_camera.m_window = m_window.get();
    }

    void OnShutdown() override
    {
        ShutdownRenderGraph();
        ShutdownPipelineAndLayout();
        ShutdownBuffers();
        ShutdownContextAndSwapchain();
    }

    void OnUpdate(Timestep ts) override
    {
        struct UniformData
        {
            glm::mat4 translate = glm::mat4(1.f);
        } uniformData;

        m_camera.Update(ts);

        uniformData.translate = m_camera.GetProjection() * m_camera.GetView();
        // uniformData.translate = glm::perspective(glm::radians(30.0f), 1600.0f/900.0f, 0.00001f, 100000000.f);

        auto uniformBufferPtr = m_device->MapBuffer(m_uniformBuffer);
        memcpy(uniformBufferPtr, &uniformData, sizeof(UniformData));
        m_device->UnmapBuffer(m_uniformBuffer);
    }

    void Render() override
    {
        static RHI::ClearValue clearValue = {.f32 = {0.3f, 0.5f, 0.7f, 1.0f}};

        auto& frame = GetCurrentFrame();

        m_device->WaitTimelineValue(frame.m_timelineValue);

        auto [width, height] = m_window->GetWindowSize();
        m_renderGraph->PassResize(m_mainPass, {width, height});

        auto commandList = m_device->CreateCommandList(RHI::QueueType::Graphics);

        commandList->Begin();
        commandList->BeginRenderPass({
            .renderGraph = m_renderGraph,
            .renderArea  = {0, 0, width, height},
            .pass        = m_mainPass,
            .loadStoreOperations{
                {
                    .clearValue     = {clearValue},
                    .loadOperation  = RHI::LoadOperation::Discard,
                    .storeOperation = RHI::StoreOperation::Store,
                },
            },
        });
        commandList->SetViewport({
            .offsetX  = 0.0f,
            .offsetY  = 0.0f,
            .width    = (float)width,
            .height   = (float)height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        });
        commandList->SetSicssor({
            .offsetX = 0,
            .offsetY = 0,
            .width   = width,
            .height  = height,
        });
        commandList->BindGraphicsPipeline(m_graphicsPipeline, {{frame.m_bindGroup}});
        commandList->BindVertexBuffers(0, {{.buffer = m_vertexBuffer, .offset = 0}});
        commandList->BindVertexBuffers(1, {{.buffer = m_vertexBuffer, .offset = sizeof(glm::vec3) * 4}});
        commandList->BindIndexBuffer({.buffer = m_indexBuffer, .offset = 0}, RHI::IndexType::uint16);
        commandList->Draw({.elementsCount = 6});
        commandList->EndRenderPass();
        commandList->End();

        static uint64_t previousSubmitValue = 0;
        previousSubmitValue = frame.m_timelineValue = m_queue->Submit({
            .waitTimelineValue = previousSubmitValue,
            .waitPipelineStage = RHI::PipelineStage::TopOfPipe,
            .commandLists      = commandList.get(),
            .swapchainToWait   = m_swapchain,
            .swapchainToSignal = m_swapchain,
        });

        auto presentResult = m_swapchain->Present();
        TL_ASSERT(presentResult == RHI::ResultCode::Success);

        m_device->CollectResources();

        AdvanceFrame();
    }

    void OnEvent(Event& event) override
    {
        switch (event.GetEventType())
        {
        case EventType::WindowResize:
            {
                auto& e   = (WindowResizeEvent&)event;
                auto  res = m_swapchain->Recreate({e.GetSize().width, e.GetSize().height});
                TL_ASSERT(res == RHI::ResultCode::Success);
            }
            break;
        default:
            {
                m_camera.ProcessEvent(event);
            }
            break;
        }
    }
};

#include <Examples-Base/Entry.hpp>

int main(int argc, const char* argv[])
{
    using namespace Examples;
    TL::Span args{argv, (size_t)argc};
    TL::MemPlumber::start();
    auto   result = Entry<Playground>(args);
    size_t memLeakCount, memLeakSize;
    TL::MemPlumber::memLeakCheck(memLeakCount, memLeakSize);
    return result;
}
