#include "Base.hpp"

class RenderPass : public RHI::PassInterface
{
public:
    RenderPass(RHI::Buffer& vertexBuffer, RHI::Buffer& indexBuffer, RHI::PipelineState& pipelineState)
        : RHI::PassInterface("Triangle", RHI::PassQueue::Graphics)
        , m_vertexBuffer(&vertexBuffer)
        , m_indexBuffer(&indexBuffer)
        , m_pipelineState(&pipelineState)
    {
    }

    void SetupAttachments(RHI::FrameGraph& builder) override
    {
        RHI::ImageAttachmentUseInfo useInfo {};
        builder.UseRenderTarget("SwapchainColor", useInfo);
    }

    void BindPassResources(RHI::ShaderResourceContext& context) override
    {
        (void)context;
    }

    void BuildCommandList(RHI::CommandList& commandList) override
    {
        RHI::Draw cmd {};
        cmd.indexBuffer        = m_indexBuffer;
        cmd.vertexBuffersCount = 1;
        cmd.vertexBuffers      = &m_vertexBuffer;
        cmd.indexedData.indexCount = 6;
        commandList.Submit(cmd);
    }

    RHI::Buffer*              m_vertexBuffer;
    RHI::Buffer*              m_indexBuffer;
    RHI::PipelineState*       m_pipelineState;
    RHI::ShaderResourceGroup* m_shaderResourceGroup;
};

class TriangleExample final : public ExampleBase
{
public:
    TriangleExample(int argc, const char* argv[])
    {
        (void)argc;
        (void)argv;

        // Create the graphics pipeline
        {
            RHI::ShaderFunction vertexShader {RHI::ShaderStage::Vertex, "VSMain", ReadBinrayFile("I:/repos/RHI/build/dev-win64/Shaders/triangle.vert.spriv")};
            RHI::ShaderFunction pixelShader {RHI::ShaderStage::Pixel, "PSMain", ReadBinrayFile("I:/repos/RHI/build/dev-win64/Shaders/triangle.pixel.spriv")};

            RHI::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
            graphicsPipelineCreateInfo.shaders.vertex                  = &vertexShader;
            graphicsPipelineCreateInfo.shaders.pixel                   = &pixelShader;
            graphicsPipelineCreateInfo.rasterizationState.lineWidth    = 1.0;
            graphicsPipelineCreateInfo.multisampleState.sampleCount    = 1;

            m_pipelineState = m_context->CreateGraphicsPipeline(graphicsPipelineCreateInfo);
        }

        {
            // clang-format off
            float vertcies[3*4] = {
                -0.5f,  0.5f, 0.0f, 
                 0.5f,  0.5f, 0.0f, 
                -0.5f, -0.5f, 0.0f, 
                 0.5f, -0.5f, 0.0f, 
            };

            uint32_t indcies[6]  = {
                0, 1, 2, 
                1, 2, 3 
            };
            // clang-format on

            RHI::ResourceAllocationInfo allocationInfo {};
            allocationInfo.usage = RHI::ResourceMemoryType::Stage;
            RHI::BufferCreateInfo createInfo {};
            createInfo.format     = RHI::Format::RGBA32Float;
            createInfo.byteSize   = sizeof(float) * 3 * 3;
            createInfo.usageFlags = RHI::BufferUsage::Vertex;
            m_vertexBuffer        = m_context->CreateBuffer(allocationInfo, createInfo);
            createInfo.format     = RHI::Format::RGBA32Float;
            createInfo.byteSize   = sizeof(float) * 3 * 3;
            createInfo.usageFlags = RHI::BufferUsage::Index;
            m_indexBuffer         = m_context->CreateBuffer(allocationInfo, createInfo);

            m_context->SetBufferContent(*m_vertexBuffer, 0, vertcies, sizeof(float) * 3 * 4);
            m_context->SetBufferContent(*m_indexBuffer, 0, indcies, sizeof(uint32_t) * 6);
        }

        {
            RHI::ResourceAllocationInfo allocationInfo {};
            allocationInfo.usage = RHI::ResourceMemoryType::DeviceLocal;
            RHI::ImageCreateInfo createInfo {};
            createInfo.arrayCount  = 1;
            createInfo.mipLevels   = 1;
            createInfo.format      = RHI::Format::RGBA8;
            createInfo.size.width  = 100;
            createInfo.size.height = 100;
            createInfo.size.depth  = 1;
            createInfo.type        = RHI::ImageType::Image2D;
            createInfo.usageFlags  = RHI::ImageUsage::ShaderRead;
            m_triangleTexture      = m_context->CreateImage(allocationInfo, createInfo);

            RHI::ImageViewCreateInfo viewCreateInfo {};
            viewCreateInfo.subresource.aspectMask     = RHI::ImageAspect::Color;
            viewCreateInfo.subresource.mipLevel       = 1;
            viewCreateInfo.subresource.layerCount     = 1;
            viewCreateInfo.subresource.baseArrayLayer = 0;
            m_triangleTextureView                     = m_triangleTexture->CreateView(viewCreateInfo);
        }

        {
            // RHI::ShaderResourceGroupLayout layout { {"Texture", RHI::ShaderResourceType::Image,
            // RHI::ShaderResourceAccess::Read, RHI::ShaderStage::Pixel, 1}}; m_resourceGroup =
            // m_shaderResourceGroupAllocator->Allocate(layout); m_resourceGroup->BindImage(0, *m_triangleTextureView);
        }

        {
            m_renderPass = std::make_unique<RenderPass>(*m_vertexBuffer, *m_indexBuffer, *m_pipelineState);
        }
    }

    void OnUpdate(double timeStep) override
    {
        (void)timeStep;

        m_scheduler->FrameBegin();

        m_scheduler->Submit(*m_renderPass);

        m_scheduler->FrameEnd();
    }

protected:
    std::unique_ptr<RHI::PipelineState> m_pipelineState;
    std::unique_ptr<RHI::Buffer>        m_vertexBuffer;
    std::unique_ptr<RHI::Buffer>        m_indexBuffer;
    std::unique_ptr<RHI::Image>         m_triangleTexture;
    std::shared_ptr<RHI::ImageView>     m_triangleTextureView;

    // std::unique_ptr<RHI::ShaderResourceGroup> m_resourceGroup;
    std::shared_ptr<RHI::FrameScheduler> m_scheduler;
    std::unique_ptr<RenderPass>          m_renderPass;
};

ENTRY_POINT(TriangleExample)