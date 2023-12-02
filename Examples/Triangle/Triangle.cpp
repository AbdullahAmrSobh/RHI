#include <Examples-Base/ExampleBase.hpp>
#include <RHI/RHI.hpp>

// clang-format off
std::vector<float> vertexData
    {
        -1.0, -1.0, 1.0, 1.0, 0.0, 1.0,
         1.0, -1.0, 0.0, 1.0, 1.0, 1.0,
         1.0,  1.0, 1.0, 0.0, 1.0, 1.0,
        -1.0,  1.0, 1.0, 1.0, 0.0, 1.0,
    };

std::vector<uint32_t> indexData = 
    { 
        0, 1, 3,
        1, 3, 2,
    };

// clang-format on

class TriangleExample final : public ExampleBase
{
public:
    TriangleExample()
        : ExampleBase("Hello, Triangle", 800, 600)
    {
    }

    template<typename T>
    RHI::Handle<RHI::Buffer> CreateBuffer(RHI::TL::Span<T> data, RHI::Flags<RHI::BufferUsage> usageFlags)
    {
        auto createInfo = RHI::BufferCreateInfo{};
        createInfo.usageFlags = usageFlags;
        createInfo.byteSize = data.size() * sizeof(T);

        auto buffer = m_bufferPool->Allocate(createInfo).GetValue();
        RHI::DeviceMemoryPtr vertexBufferPtr = m_context->MapResource(buffer);
        RHI_ASSERT(vertexBufferPtr != nullptr);
        memcpy(vertexBufferPtr, data.data(), data.size() * sizeof(float));
        m_context->Unmap(buffer);

        return buffer;
    }

    void SetupPipelines(RHI::Handle<RHI::BindGroupLayout> bindGroupLayout)
    {
        auto pipelineLayout = m_context->CreatePipelineLayout({ bindGroupLayout });

        auto shaderCode = ReadBinaryFile("./Resources/Shaders/triangle.spv");

        RHI::ShaderModuleCreateInfo createInfo{};
        createInfo.code = shaderCode.data();
        createInfo.size = shaderCode.size();

        auto shaderModule = m_context->CreateShaderModule(createInfo);

        RHI::GraphicsPipelineCreateInfo psoCreateInfo{};
        psoCreateInfo.inputAssemblerState.attributes = {
            {
                .location = 0,
                .binding = 0,
                .format = RHI::Format::RG32_FLOAT,
                .offset = 0,
            },
            {
                .location = 1,
                .binding = 0,
                .format = RHI::Format::RGBA32_FLOAT,
                .offset = RHI::GetFormatInfo(RHI::Format::RG32_FLOAT).bytesPerBlock,
            },
        };
        psoCreateInfo.inputAssemblerState.bindings = {
            {
                .binding = 0,
                .stride = uint32_t(RHI::GetFormatInfo(RHI::Format::RG32_FLOAT).bytesPerBlock + RHI::GetFormatInfo(RHI::Format::RGBA32_FLOAT).bytesPerBlock),
                .stepRate = RHI::PipelineVertexInputRate::PerVertex,
            }
        };
        psoCreateInfo.vertexShaderModule = shaderModule.get();
        psoCreateInfo.vertexShaderName = "VSMain";
        psoCreateInfo.pixelShaderModule = shaderModule.get();
        psoCreateInfo.pixelShaderName = "PSMain";
        psoCreateInfo.topologyMode = RHI::PipelineTopologyMode::Triangles;
        psoCreateInfo.rasterizationState.cullMode = RHI::PipelineRasterizerStateCullMode::None;
        psoCreateInfo.renderTargetLayout = { { RHI::Format::BGRA8_UNORM }, RHI::Format::Unknown, RHI::Format::Unknown };
        psoCreateInfo.depthStencilState.depthTestEnable = false;
        psoCreateInfo.depthStencilState.depthWriteEnable = false;
        psoCreateInfo.layout = pipelineLayout;
        psoCreateInfo.renderTargetLayout.colorAttachmentsFormats = { RHI::Format::BGRA8_UNORM };
        psoCreateInfo.renderTargetLayout.depthAttachmentFormat = RHI::Format::D32;
        psoCreateInfo.colorBlendState.blendStates = {
            {
                .blendEnable = true,
                .colorBlendOp = RHI::BlendEquation::Add,
                .srcColor = RHI::BlendFactor::One,
                .dstColor = RHI::BlendFactor::Zero,
                .alphaBlendOp = RHI::BlendEquation::Add,
                .srcAlpha = RHI::BlendFactor::One,
                .dstAlpha = RHI::BlendFactor::Zero,
            }
        };

        m_pipelineState = m_context->CreateGraphicsPipeline(psoCreateInfo);
    }

    void OnInit(WindowInfo windowInfo) override
    {
        (void)windowInfo;
        // create resources pool
        {
            RHI::ResourcePoolCreateInfo createInfo{};
            createInfo.heapType = RHI::MemoryType::CPU;
            createInfo.allocationAlgorithm = RHI::AllocationAlgorithm::Linear;
            createInfo.blockSize = 10 * RHI::AllocationSizeConstants::KB;
            createInfo.minBlockAlignment = alignof(uint64_t);

            m_bufferPool = m_context->CreateResourcePool(createInfo);
        }

        // create buffer resource
        {
            m_vertexBuffer = CreateBuffer<float>({ vertexData.data(), vertexData.size() }, RHI::BufferUsage::Vertex);
            m_indexBuffer = CreateBuffer<uint32_t>({ indexData.data(), indexData.size() }, RHI::BufferUsage::Index);

            struct UniformData
            {
                float r, g, b, a;
            };

            UniformData data{};
            data.r = 1.0f;
            data.g = 0.3f;
            data.b = 0.4f;
            data.a = 0.7f;
            m_uniformData = CreateBuffer<UniformData>(data, RHI::BufferUsage::Uniform);
        }

        // // create image resource
        // {
        //     auto imageData = LoadImage("Resources/Images/image.png");

        //     RHI::ImageCreateInfo createInfo{};
        //     createInfo.usageFlags = RHI::ImageUsage::ShaderResource;
        //     createInfo.type = RHI::ImageType::Image2D;
        //     createInfo.size.width = imageData.width;
        //     createInfo.size.height = imageData.height;
        //     createInfo.size.depth = imageData.depth;
        //     createInfo.format = RHI::Format::BGRA8_UNORM;
        //     createInfo.mipLevels = 1;
        //     createInfo.arrayCount = 1;
        //     m_image = m_bufferPool->Allocate(createInfo).GetValue();

        //     auto dataSize = imageData.width * imageData.height * imageData.channels * imageData.bytesPerChannel;
        //     auto cpyCmd = m_commandListAllocator->Allocate();
        //     cpyCmd->Begin();
        //     cpyCmd->Submit(RHI::CopyBufferToImageDescriptor{
        //         .srcBuffer = m_stagingBuffer,
        //         .srcOffset = 0,
        //         .srcBytesPerRow = 0,
        //         .srcBytesPerImage = dataSize,
        //         .srcSize = { .width = imageData.width, .height = imageData.height, .depth = imageData.depth },
        //         .dstImage = m_image,
        //         .dstOffset = { 0, 0, 0 } });
        //     cpyCmd->End();
        // }

        // create shader bind group layout
        auto bindGroupLayout = m_context->CreateBindGroupLayout({ RHI::ShaderBinding{ RHI::ShaderBindingType::Buffer, RHI::ShaderBindingAccess::OnlyRead, 1, RHI::ShaderStage::Pixel } });
        SetupPipelines(bindGroupLayout);

        // create shader bind group
        m_bindGroupAllocator = m_context->CreateBindGroupAllocator();
        m_bindGroup = m_bindGroupAllocator->AllocateBindGroups(bindGroupLayout).front();

        m_commandListAllocator = m_context->CreateCommandListAllocator(RHI::QueueType::Graphics);

        // create frame graph
        {
            RHI::PassCreateInfo createInfo{};
            createInfo.name = "RenderPass";
            createInfo.type = RHI::QueueType::Graphics;
            m_renderpass = m_frameScheduler->CreatePass(createInfo);
        }

        {
            m_renderpass->Begin();

            // setup attachments
            RHI::ImageCreateInfo createInfo{};
            createInfo.usageFlags = RHI::ImageUsage::Depth;
            createInfo.size.width = 800;
            createInfo.size.height = 600;
            createInfo.size.depth = 1;
            createInfo.format = RHI::Format::D32;
            createInfo.type = RHI::ImageType::Image2D;

            RHI::ImageAttachmentUseInfo useInfo{};
            useInfo.usage = RHI::AttachmentUsage::RenderTarget;
            useInfo.subresource.imageAspects = RHI::ImageAspect::Color;
            useInfo.loadStoreOperations.loadOperation = RHI::ImageLoadOperation::Discard;
            useInfo.loadStoreOperations.storeOperation = RHI::ImageStoreOperation::Store;
            useInfo.clearValue.color = { 0.3f, 0.6f, 0.9f, 1.0f };
            m_renderpass->ImportSwapchainImageResource("color-attachment", m_swapchain.get(), useInfo);

            // auto textureAttachment = m_renderpass->ImportImageResource("texture", m_image, useInfo);

            RHI::BufferAttachmentUseInfo bufferUseInfo{};
            bufferUseInfo.access = RHI::AttachmentAccess::Read;
            bufferUseInfo.usage = RHI::AttachmentUsage::ShaderResource;
            auto uniformBuffer = m_renderpass->ImportBufferResource("uniform-buffer", m_uniformData, bufferUseInfo);

            // setup bind elements
            m_renderpass->End();

            RHI::BindGroupData BindGroupData{};
            BindGroupData.BindBuffers(0u, uniformBuffer);
            // BindGroupData.BindImages(1u, textureAttachment);
            m_bindGroupAllocator->Update(m_bindGroup, BindGroupData);

            m_frameScheduler->Submit(*m_renderpass);
        }
    }

    void OnShutdown() override
    {
        m_context->Free(m_pipelineState);

        m_bufferPool->Free(m_vertexBuffer);

        m_bufferPool->Free(m_indexBuffer);

        // m_bufferPool->Free(m_image);
    }

    void OnUpdate() override
    {
        m_frameScheduler->Begin();

        m_commandListAllocator->Flush();

        auto cmd = m_commandListAllocator->Allocate();

        cmd->Begin(*m_renderpass);

        cmd->SetViewport({
            .width = 800,
            .height = 600,
            .minDepth = 0.0,
            .maxDepth = 1.0,
        });

        cmd->SetSicssor({
            .width = 800,
            .height = 600,
        });

        cmd->Submit({
            .pipelineState = m_pipelineState,
            .bindGroups = m_bindGroup,
            .vertexBuffers = m_vertexBuffer,
            .indexBuffers = m_indexBuffer,
            .parameters = { .elementCount = 6 },
        });

        cmd->End();

        m_renderpass->Execute(cmd);

        m_frameScheduler->End();
    }

private:
    std::unique_ptr<RHI::ResourcePool> m_bufferPool;

    std::unique_ptr<RHI::ResourcePool> m_imagePool;

    std::unique_ptr<RHI::BindGroupAllocator> m_bindGroupAllocator;

    std::unique_ptr<RHI::CommandListAllocator> m_commandListAllocator;

    RHI::Handle<RHI::BindGroup> m_bindGroup;

    RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;

    RHI::Handle<RHI::GraphicsPipeline> m_pipelineState;

    RHI::Handle<RHI::Image> m_image;

    RHI::Handle<RHI::Buffer> m_uniformData;

    RHI::Handle<RHI::Buffer> m_stagingBuffer;

    RHI::Handle<RHI::Buffer> m_vertexBuffer;

    RHI::Handle<RHI::Buffer> m_indexBuffer;

    std::unique_ptr<RHI::Pass> m_renderpass;
};

EXAMPLE_ENTRY_POINT(TriangleExample)
