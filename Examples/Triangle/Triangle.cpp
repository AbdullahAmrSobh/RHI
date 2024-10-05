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

#include "Camera.hpp"

using namespace Examples;

class Playground final : public ApplicationBase
{
public:
    Playground()
        : ApplicationBase("Playground", 1600, 900)
    {
    }

    RHI::Context* m_context;
    RHI::Swapchain* m_swapchain;
    RHI::Queue* m_queue;

    struct PerFrame
    {
        RHI::Fence* m_fence;
        RHI::CommandPool* m_commandPool;
        RHI::Handle<RHI::BindGroup> m_bindGroup;
    };

    uint32_t m_frameIndex = 0;
    PerFrame m_perFrameData[2] = {};

    RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline> m_graphicsPipeline;

    RHI::RenderGraph* m_renderGraph;
    RHI::Handle<RHI::Pass> m_mainPass;
    RHI::Handle<RHI::ImageAttachment> m_colorAttachment;

    RHI::Handle<RHI::Buffer> m_vertexBuffer;
    RHI::Handle<RHI::Buffer> m_indexBuffer;
    RHI::Handle<RHI::Buffer> m_uniformBuffer;

    Camera m_camera;

    PerFrame& AdvanceFrame()
    {
        m_frameIndex += 1;
        m_frameIndex %= 2;
        return m_perFrameData[m_frameIndex];
    }

    PerFrame& GetCurrentFrame()
    {
        return m_perFrameData[m_frameIndex];
    }

    void InitContextAndSwapchain()
    {
        RHI::ApplicationInfo appInfo{
            .applicationName = "Example",
            .applicationVersion = {0,  1, 0},
            .engineName = "Forge",
            .engineVersion = { 0, 1, 0}
        };
        m_context = RHI::CreateVulkanContext(appInfo).release();

        auto [width, height] = m_window->GetWindowSize();
        RHI::SwapchainCreateInfo swapchainInfo{
            .name = "Swapchain",
            .imageSize = { width, height },
            .imageUsage = RHI::ImageUsage::Color,
            .imageFormat = RHI::Format::RGBA8_UNORM,
            .minImageCount = 2,
            .presentMode = RHI::SwapchainPresentMode::Fifo,
            .win32Window = { m_window->GetNativeHandle() }
        };

        m_swapchain = m_context->CreateSwapchain(swapchainInfo).release();

        m_queue = m_context->GetQueue(RHI::QueueType::Graphics);

        for (auto& frame : m_perFrameData)
        {
            frame.m_commandPool = m_context->CreateCommandPool(RHI::CommandPoolFlags::Transient).release();
            frame.m_fence = m_context->CreateFence().release();
        }
    }

    void ShutdownContextAndSwapchain()
    {
        for (auto& frame : m_perFrameData)
        {
            delete frame.m_commandPool;
            delete frame.m_fence;
        }
        delete m_swapchain;
        delete m_context;
    }

    void InitPipelineAndLayout()
    {
        TL::Vector<uint32_t> spv;
        auto spvBlock = TL::ReadBinaryFile("./Shaders/Basic.spv");
        spv.resize(spvBlock.size / 4);
        memcpy(spv.data(), spvBlock.ptr, spvBlock.size);
        auto shaderModule = m_context->CreateShaderModule(spv);
        TL::Allocator::Release(spvBlock, alignof(char));

        // clang-format off
        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{
            .name = "BGL-ViewUB",
            .bindings =
            {
                {
                    .type = RHI::BindingType::UniformBuffer,
                    .access = RHI::Access::Read,
                    .arrayCount = 1,
                    .stages = RHI::ShaderStage::Pixel | RHI::ShaderStage::Vertex
                },
                // {
                //     .type = RHI::BindingType::DynamicUniformBuffer,
                //     .access = RHI::Access::Read,
                //     .arrayCount = 1,
                //     .stages = RHI::ShaderStage::Pixel | RHI::ShaderStage::Vertex
                // },
            }
        };
        auto bindGroupLayout = m_context->CreateBindGroupLayout(bindGroupLayoutCI);
        // clang-format on

        RHI::PipelineLayoutCreateInfo layoutCI{ .name = "graphics-pipeline-layout", .layouts = { bindGroupLayout } };
        m_pipelineLayout = m_context->CreatePipelineLayout(layoutCI);

        RHI::GraphicsPipelineCreateInfo pipelineCI{};
        pipelineCI.name = "Basic";
        pipelineCI.vertexShaderName = "VSMain";
        pipelineCI.vertexShaderModule = shaderModule.get();
        pipelineCI.pixelShaderName = "PSMain";
        pipelineCI.pixelShaderModule = shaderModule.get();
        pipelineCI.layout = m_pipelineLayout;
        pipelineCI.inputAssemblerState.bindings = {
            {
             .binding = 0,
             .stride = sizeof(glm::vec3),
             .stepRate = RHI::PipelineVertexInputRate::PerVertex,
             },
            {
             .binding = 1,
             .stride = sizeof(glm::vec4),
             .stepRate = RHI::PipelineVertexInputRate::PerVertex,
             }
        };
        pipelineCI.inputAssemblerState.attributes = {
            {
             .location = 0,
             .binding = 0,
             .format = RHI::Format::RGB32_FLOAT,
             .offset = 0,
             },
            {
             .location = 1,
             .binding = 1,
             .format = RHI::Format::RGBA32_FLOAT,
             .offset = 0,
             }
        };

        pipelineCI.renderTargetLayout.colorAttachmentsFormats = RHI::Format::RGBA8_UNORM;

        pipelineCI.colorBlendState.blendStates = {
            {
             .blendEnable = true,
             .colorBlendOp = RHI::BlendEquation::Add,
             .srcColor = RHI::BlendFactor::SrcAlpha,
             .dstColor = RHI::BlendFactor::OneMinusSrcAlpha,
             .alphaBlendOp = RHI::BlendEquation::Add,
             .srcAlpha = RHI::BlendFactor::One,
             .dstAlpha = RHI::BlendFactor::Zero,
             .writeMask = RHI::ColorWriteMask::All,
             }
        };

        pipelineCI.topologyMode = RHI::PipelineTopologyMode::Triangles;
        pipelineCI.rasterizationState.cullMode = RHI::PipelineRasterizerStateCullMode::None;
        pipelineCI.rasterizationState.fillMode = RHI::PipelineRasterizerStateFillMode::Triangle;
        pipelineCI.rasterizationState.frontFace = RHI::PipelineRasterizerStateFrontFace::CounterClockwise;
        pipelineCI.rasterizationState.lineWidth = 1.0;
        pipelineCI.multisampleState.sampleCount = RHI::SampleCount::Samples1;
        pipelineCI.multisampleState.sampleShading = false;
        pipelineCI.depthStencilState.depthTestEnable = true;
        pipelineCI.depthStencilState.depthWriteEnable = true;
        pipelineCI.depthStencilState.compareOperator = RHI::CompareOperator::Less;
        pipelineCI.depthStencilState.stencilTestEnable = false;
        m_graphicsPipeline = m_context->CreateGraphicsPipeline(pipelineCI);

        // init and update bind groups

        // clang-format off
        for (auto& frame : m_perFrameData)
        {
            frame.m_bindGroup = m_context->CreateBindGroup(bindGroupLayout);
            RHI::BindGroupUpdateInfo bindGroupUpdateInfo{
                .buffers =
                {
                    {
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .buffer = m_uniformBuffer,
                        .subregions = {}
                    },
                    // {
                    //     .dstBinding = 1,
                    //     .dstArrayElement = 0,
                    //     .buffer = m_uniformBuffer2,
                    //     .subregions = { RHI::BufferSubregion{ 0, 256 } }
                    // },
                }
            };
            m_context->UpdateBindGroup(frame.m_bindGroup, bindGroupUpdateInfo);
            // clang-format on
        }

        m_context->DestroyBindGroupLayout(bindGroupLayout);
    }

    void ShutdownPipelineAndLayout()
    {
        for (auto& frame : m_perFrameData)
        {
            m_context->DestroyBindGroup(frame.m_bindGroup);
        }
        m_context->DestroyGraphicsPipeline(m_graphicsPipeline);
        m_context->DestroyPipelineLayout(m_pipelineLayout);
    }

    void InitRenderGraph()
    {
        auto [width, height] = m_window->GetWindowSize();
        m_renderGraph = m_context->CreateRenderGraph().release();

        RHI::PassCreateInfo passCI{
            .name = "main-pass",
            .flags = RHI::PassFlags::Graphics,
        };
        m_mainPass = m_renderGraph->CreatePass(passCI);

        m_colorAttachment = m_renderGraph->ImportSwapchain("main-output", *m_swapchain);

        RHI::ImageViewInfo viewInfo;
        viewInfo.type = RHI::ImageViewType::View2D;
        viewInfo.subresources.imageAspects = RHI::ImageAspect::Color;
        viewInfo.subresources.mipBase = 0;
        viewInfo.subresources.mipLevelCount = 1;
        viewInfo.subresources.arrayBase = 0;
        viewInfo.subresources.arrayCount = 1;
        viewInfo.swizzle.r = RHI::ComponentSwizzle::Identity;
        viewInfo.swizzle.g = RHI::ComponentSwizzle::Identity;
        viewInfo.swizzle.b = RHI::ComponentSwizzle::Identity;
        viewInfo.swizzle.a = RHI::ComponentSwizzle::Identity;
        m_renderGraph->PassUseImage(m_mainPass, m_colorAttachment, viewInfo, RHI::ImageUsage::Color, RHI::ShaderStage::None, RHI::Access::None);
        m_renderGraph->PassResize(m_mainPass, { width, height });
        m_context->CompileRenderGraph(*m_renderGraph);
    }

    void ShutdownRenderGraph()
    {
        delete m_renderGraph;
    }

    void InitBuffers()
    {
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
        size_t indexBufferSize = sizeof(uint16_t) * 6;

        RHI::BufferCreateInfo vertexBufferCI{
            .name = "vertex-buffer",
            .heapType = RHI::MemoryType::GPUShared,
            .usageFlags = RHI::BufferUsage::Vertex,
            .byteSize = vertexBufferSize,
        };
        m_vertexBuffer = m_context->CreateBuffer(vertexBufferCI).GetValue();

        auto vertexBufferPtr = m_context->MapBuffer(m_vertexBuffer);
        memcpy(vertexBufferPtr, positionData, sizeof(glm::vec3) * 4);
        memcpy((char*)vertexBufferPtr + sizeof(glm::vec3) * 4, colorData, sizeof(glm::vec4) * 4);
        m_context->UnmapBuffer(m_vertexBuffer);

        RHI::BufferCreateInfo indexBufferCI{
            .name = "index-buffer",
            .heapType = RHI::MemoryType::GPUShared,
            .usageFlags = RHI::BufferUsage::Index,
            .byteSize = indexBufferSize,
        };
        m_indexBuffer = m_context->CreateBuffer(indexBufferCI).GetValue();

        auto indexBufferPtr = m_context->MapBuffer(m_indexBuffer);
        memcpy(indexBufferPtr, indexData, sizeof(uint16_t) * 6);
        m_context->UnmapBuffer(m_indexBuffer);

        // uniform buffer data
        struct UniformData
        {
            glm::mat4 translate;
        };

        UniformData uniformData{ glm::mat4(1.f) };
        RHI::BufferCreateInfo uniformBufferCI{
            .name = "uniform-buffer",
            .heapType = RHI::MemoryType::GPUShared,
            .usageFlags = RHI::BufferUsage::Uniform,
            .byteSize = sizeof(UniformData),
        };
        m_uniformBuffer = m_context->CreateBuffer(uniformBufferCI).GetValue();
        auto uniformBufferPtr = m_context->MapBuffer(m_uniformBuffer);
        memcpy(uniformBufferPtr, &uniformData, sizeof(UniformData));
        m_context->UnmapBuffer(m_uniformBuffer);
    }

    void ShutdownBuffers()
    {
        m_context->DestroyBuffer(m_uniformBuffer);
        m_context->DestroyBuffer(m_vertexBuffer);
        m_context->DestroyBuffer(m_indexBuffer);
    }

    virtual void OnInit()
    {
        InitContextAndSwapchain();
        InitBuffers();
        InitPipelineAndLayout();
        InitRenderGraph();

        auto [width, height] = m_window->GetWindowSize();
        m_camera.SetPerspective(30.0f, (float)width / (float)height, 0.00001f, 100000.0f);
        m_camera.m_window = m_window.get();
    }

    virtual void OnShutdown()
    {
        ShutdownContextAndSwapchain();
        ShutdownBuffers();
        ShutdownPipelineAndLayout();
        ShutdownRenderGraph();
    }

    virtual void OnUpdate(Timestep ts)
    {
        struct UniformData
        {
            glm::mat4 translate = glm::mat4(1.f);
        } uniformData;

        m_camera.Update(ts);

        uniformData.translate = m_camera.GetProjection() * m_camera.GetView();
        // uniformData.translate = glm::perspective(glm::radians(30.0f), 1600.0f/900.0f, 0.00001f, 100000000.f);

        auto uniformBufferPtr = m_context->MapBuffer(m_uniformBuffer);
        memcpy(uniformBufferPtr, &uniformData, sizeof(UniformData));
        m_context->UnmapBuffer(m_uniformBuffer);
    }

    virtual void Render()
    {
        auto frame = GetCurrentFrame();

        auto res = m_swapchain->AcquireNextImage();
        TL_ASSERT(res == RHI::ResultCode::Success);

        if (frame.m_fence->GetState() != RHI::FenceState::Signaled)
        {
            [[maybe_unused]] auto result = frame.m_fence->Wait(UINT64_MAX);
            // TL_ASSERT(result);
        }

        frame.m_fence->Reset();
        frame.m_commandPool->Reset();

        auto commandList = frame.m_commandPool->Allocate(RHI::QueueType::Graphics, RHI::CommandListLevel::Primary);

        auto [width, height] = m_renderGraph->GetPassSize(m_mainPass);

        RHI::CommandListBeginInfo renderPassBeginInfo{
            .renderGraph = m_renderGraph,
            .pass = m_mainPass,
            .loadStoreOperations = { {
                .clearValue = { .f32 = RHI::ColorValue<float>{ 0.3f, 0.5f, 0.7f, 1.0f } },
                .loadOperation = RHI::LoadOperation::Discard,
                .storeOperation = RHI::StoreOperation::Store,
            } }
        };

        commandList->Begin(renderPassBeginInfo);
        // commandList->Begin();

        commandList->SetViewport(RHI::Viewport{
            .offsetX = 0.0f,
            .offsetY = 0.0f,
            .width = (float)width,
            .height = (float)height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        });

        commandList->SetSicssor(RHI::Scissor{
            .offsetX = 0,
            .offsetY = 0,
            .width = width,
            .height = height,
        });
        commandList->BindGraphicsPipeline(m_graphicsPipeline, RHI::BindGroupBindingInfo{ frame.m_bindGroup, {} });
        commandList->BindVertexBuffers(0, RHI::BufferBindingInfo{ .buffer = m_vertexBuffer, .offset = 0 });
        commandList->BindVertexBuffers(1, RHI::BufferBindingInfo{ .buffer = m_vertexBuffer, .offset = sizeof(glm::vec3) * 4 });
        commandList->BindIndexBuffer(RHI::BufferBindingInfo{ .buffer = m_indexBuffer, .offset = 0 }, RHI::IndexType::uint16);

        RHI::DrawInfo drawInfo{};
        drawInfo.parameters = { 6, 1, 0, 0, 0 };
        commandList->Draw(drawInfo);

        commandList->End();

        RHI::SubmitInfo submitInfo{
            .waitSemaphores = RHI::SemaphoreSubmitInfo{0,  RHI::PipelineStage::None, m_swapchain->GetWaitSemaphore()  },
            .commandLists = commandList.get(),
            .signalSemaphores = RHI::SemaphoreSubmitInfo{ 0, RHI::PipelineStage::None, m_swapchain->GetSignalSemaphore()},
        };
        m_queue->Submit(submitInfo, frame.m_fence);

        AdvanceFrame();

        auto presentResult = m_swapchain->Present();
        TL_ASSERT(presentResult == RHI::ResultCode::Success);
    }

    virtual void OnEvent(Event& event)
    {
        m_camera.ProcessEvent(event);
    }
};

#include <Examples-Base/Entry.hpp>

int main(int argc, const char* argv[])
{
    using namespace Examples;
    TL::Span args{ argv, (size_t)argc };
    TL::MemPlumber::start();
    auto result = Entry<Playground>(args);
    size_t memLeakCount, memLeakSize;
    TL::MemPlumber::memLeakCheck(memLeakCount, memLeakSize);
    return result;
}
