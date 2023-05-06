#include "Base.hpp"

class RenderPass : public RHI::Pass
{
public:
    RenderPass(RHI::Buffer&              vertexBuffer,
               RHI::Buffer&              indexBuffer,
               RHI::PipelineState&       pipelineState,
               RHI::ShaderResourceGroup& shaderResourceGroup)
        : RHI::Pass("Triangle", RHI::PassQueue::Graphics)
        , m_vertexBuffer(&vertexBuffer)
        , m_indexBuffer(&indexBuffer)
        , m_pipelineState(&pipelineState)
        , m_shaderResourceGroup(&shaderResourceGroup)
    {
    }

    void SetupAttachments(RHI::FrameGraphBuilder builder) override
    {
    }

    void BindCompiledResources(RHI::CompileContext context) override
    {
        (void)context;
    }

    void BuildCommandLists(uint32_t dispatchIndex, RHI::CommandList& commandList) override
    {
        assert(dispatchIndex == 0);

        commandList.SetPipelineState(*m_pipelineState);
        commandList.BindShaderResourceGroup(*m_shaderResourceGroup);

        RHI::DrawIndexedData drawData {};
        drawData.instanceBuffer   = nullptr;
        drawData.vertexBuffer     = m_vertexBuffer;
        drawData.indexBuffer      = m_indexBuffer;
        drawData.instanceCount    = 1;
        drawData.indexCount     = 6;
        drawData.firstIndexOffset = 0;
        commandList.DrawIndexed(drawData);
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
            // auto vertexShader = RHI::ShaderFunction(RHI::ShaderType::Vertex, ReadBinrayFile("./Shaders/TriangleExample/vertex.spv"));
            // auto pixelShader  = RHI::ShaderFunction(RHI::ShaderType::Pixel, ReadBinrayFile("./Shaders/TriangleExample/pixel.spv"));

            // auto graphicsPipelineCreateInfo = RHI::GraphicsPipelineCreateInfo(RHI::GraphicsPipelineShaderTypes(vertexShader,
            // pixelShader));

            // m_pipelineStateObject = m_context->CreateGraphicsPipeline(graphicsPipelineCreateInfo);
        }

        {
            // clang-format off
            float    vertcies[3*4] = {
                -0.5,  0.5, 0.0, 
                 0.5,  0.5, 0.0, 
                -0.5, -0.5, 0.0, 
                 0.5, -0.5, 0.0, 
            };

            uint32_t indcies[6]  = {
                0, 1, 2, 
                1, 2, 3 
            };
            // clang-format on

            RHI::ResourceAllocationInfo allocationInfo {};
            RHI::BufferCreateInfo       createInfo {};
            createInfo.format     = RHI::Format::R32G32B32_FLOAT;
            createInfo.byteSize   = sizeof(float) * 3 * 3;
            createInfo.usageFlags = RHI::BufferUsage::Vertex;
            m_vertexBuffer        = m_context->CreateBuffer(allocationInfo, createInfo);
            createInfo.format     = RHI::Format::R32G32B32_FLOAT;
            createInfo.byteSize   = sizeof(float) * 3 * 3;
            createInfo.usageFlags = RHI::BufferUsage::Index;
            m_indexBuffer         = m_context->CreateBuffer(allocationInfo, createInfo);

            {
                RHI::ResultCode result = m_context->SetBufferData(*m_vertexBuffer, 0, vertcies, sizeof(float) * 3 * 4);
                assert(result == RHI::ResultCode::Success);
            }

            {
                RHI::ResultCode result = m_context->SetBufferData(*m_vertexBuffer, 0, indcies, sizeof(uint32_t) * 6);
                assert(result == RHI::ResultCode::Success);
            }
        }

        {
            RHI::ResourceAllocationInfo allocationInfo {};
            RHI::ImageCreateInfo        createInfo {};
            m_triangleTexture = m_context->CreateImage(allocationInfo, createInfo);

            RHI::ImageViewCreateInfo viewCreateInfo {};
            m_triangleTextureView = m_triangleTexture->CreateView(viewCreateInfo);
        }

        {
            RHI::ShaderResourceGroupLayout layout {
                {"Texture", RHI::ShaderBindingResourceType::Image, RHI::ShaderBindingResourceAccess::Read, RHI::ShaderType::Pixel, 1}};
            // m_resourceGroup = m_shaderResourceGroupAllocator->Allocate(layout);
            // m_resourceGroup->BindImage(0, *m_triangleTextureView);
        }

        {
            m_renderPass = std::make_unique<RenderPass>(*m_vertexBuffer, *m_indexBuffer, *m_pipelineStateObject, *m_resourceGroup);
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
    std::unique_ptr<RHI::PipelineState>       m_pipelineStateObject;
    std::unique_ptr<RHI::Buffer>              m_vertexBuffer;
    std::unique_ptr<RHI::Buffer>              m_indexBuffer;
    std::unique_ptr<RHI::Image>               m_triangleTexture;
    std::shared_ptr<RHI::ImageView>           m_triangleTextureView;
    std::unique_ptr<RHI::ShaderResourceGroup> m_resourceGroup;
    std::unique_ptr<RenderPass>               m_renderPass;
};

ENTRY_POINT(TriangleExample)