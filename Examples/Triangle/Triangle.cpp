#include <Examples-Base/ApplicationBase.hpp>

#include <Assets/Importer.hpp>

#include <RPI/Renderer.hpp>
#include <RPI/ConstantBuffer.hpp>

#include <tracy/Tracy.hpp>

#include "Camera.hpp"

#include "TL/FileSystem/FileSystem.hpp"

using namespace Examples;

class RenderImpl final : public RPI::Renderer
{
public:
    RHI::Handle<RHI::Buffer> m_uniformBuffer;
    RHI::Handle<RHI::BindGroup> m_bindGroup;

    RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline> m_graphicsPipeline;
    RHI::Handle<RHI::Pass> m_pass;

    RPI::ResultCode OnInit() override
    {
        auto renderGraph = m_renderGraph.get();
        auto context = m_context.get();

        RHI::PassCreateInfo createInfo{
            .name = "Main",
            .flags = RHI::PassFlags::Graphics,
        };
        m_pass = renderGraph->CreatePass(createInfo);
        // clang-format off
        RHI::ImageViewInfo viewInfo
        {
            .type = RHI::ImageViewType::View2D,
            .subresources =
            {
                .imageAspects = RHI::ImageAspect::Color,
                .mipBase = 0,
                .mipLevelCount = 1,
                .arrayBase = 0,
                .arrayCount = 1,
            },
            .swizzle =
            {
                .r = RHI::ComponentSwizzle::Identity,
                .g = RHI::ComponentSwizzle::Identity,
                .b = RHI::ComponentSwizzle::Identity,
                .a = RHI::ComponentSwizzle::Identity
            }
        };
        // clang-format on
        renderGraph->PassUseImage(m_pass, m_outputAttachment, viewInfo, RHI::ImageUsage::Color, RHI::ShaderStage::None, RHI::Access::None);
        renderGraph->PassResize(m_pass, { 1600, 1200 });
        context->CompileRenderGraph(*renderGraph);

        TL::Vector<uint32_t> spv;
        auto spvBlock = TL::ReadBinaryFile("./Shaders/Basic.spv");
        spv.resize(spvBlock.size / 4);
        memcpy(spv.data(), spvBlock.ptr, spvBlock.size);
        auto shaderMoudule = context->CreateShaderModule(spv);

        RHI::BufferCreateInfo uniformBufferCI{};
        uniformBufferCI.name = "UniformBuffer";
        uniformBufferCI.heapType = RHI::MemoryType::GPULocal;
        uniformBufferCI.usageFlags = RHI::BufferUsage::Uniform;
        uniformBufferCI.byteSize = sizeof(glm::vec4);
        m_uniformBuffer = context->CreateBuffer(uniformBufferCI).GetValue();

        auto ptr = context->MapBuffer(m_uniformBuffer);
        glm::vec4 color{ 1.0, 0.5, 0.6, 1.0 };
        memcpy(ptr, &color, sizeof(glm::vec4));

        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{};
        bindGroupLayoutCI.bindings[0].type = RHI::BindingType::UniformBuffer;
        bindGroupLayoutCI.bindings[0].access = RHI::Access::Read;
        bindGroupLayoutCI.bindings[0].arrayCount = 1;
        bindGroupLayoutCI.bindings[0].stages = RHI::ShaderStage::Pixel;
        auto bindGroupLayout = context->CreateBindGroupLayout(bindGroupLayoutCI);

        m_bindGroup = context->CreateBindGroup(bindGroupLayout);
        context->UpdateBindGroup(m_bindGroup, { RHI::BindGroupUpdateInfo(0, 0, m_uniformBuffer) });

        RHI::PipelineLayoutCreateInfo pipelineLayoutCI{ bindGroupLayout };
        m_pipelineLayout = context->CreatePipelineLayout(pipelineLayoutCI);

        context->DestroyBindGroupLayout(bindGroupLayout);

        // clang-format off
        auto defaultBlendState = RHI::ColorAttachmentBlendStateDesc{
            .blendEnable = true,
            .colorBlendOp = RHI::BlendEquation::Add,
            .srcColor = RHI::BlendFactor::SrcAlpha,
            .dstColor = RHI::BlendFactor::OneMinusSrcAlpha,
            .alphaBlendOp = RHI::BlendEquation::Add,
            .srcAlpha = RHI::BlendFactor::One,
            .dstAlpha = RHI::BlendFactor::Zero,
            .writeMask = RHI::ColorWriteMask::All,
        };

        RHI::GraphicsPipelineCreateInfo pipelineCI
        {
            .name = "Basic",
            .vertexShaderName = "VSMain",
            .vertexShaderModule = shaderMoudule.get(),
            .pixelShaderName = "PSMain",
            .pixelShaderModule = shaderMoudule.get(),
            .layout = m_pipelineLayout,
            .inputAssemblerState = {},
            .renderTargetLayout =
                {
                    .colorAttachmentsFormats = { RHI::Format::RGBA8_UNORM },
                },
            .colorBlendState =
                {
                    .blendStates =
                    {
                        defaultBlendState,
                    },
                    .blendConstants = {}
                },
            .topologyMode = RHI::PipelineTopologyMode::Triangles,
            .rasterizationState =
                {
                    .cullMode = RHI::PipelineRasterizerStateCullMode::None,
                    .fillMode = RHI::PipelineRasterizerStateFillMode::Triangle,
                    .frontFace = RHI::PipelineRasterizerStateFrontFace::CounterClockwise,
                    .lineWidth = 1.0,
                },
            .multisampleState =
                {
                    .sampleCount = RHI::SampleCount::Samples1,
                    .sampleShading = false,
                },
            .depthStencilState =
                {
                    .depthTestEnable = true,
                    .depthWriteEnable = true,
                    .compareOperator = RHI::CompareOperator::Less,
                    .stencilTestEnable = false,
                },
        };
        m_graphicsPipeline = context->CreateGraphicsPipeline(pipelineCI);
        // clang-format on

        return RPI::ResultCode::Sucess;
    }

    void OnShutdown() override
    {
    }

    void OnRender() override
    {
        auto& frame = m_frameRingbuffer.Get();

        auto commandList = frame.m_commandPool->Allocate(RHI::QueueType::Graphics, RHI::CommandListLevel::Primary);

        RHI::CommandListBeginInfo beginInfo{};
        beginInfo.renderGraph = m_renderGraph.get();
        beginInfo.pass = m_pass;
        beginInfo.loadStoreOperations = {
            {{}, RHI::LoadOperation::Discard, RHI::StoreOperation::Store}
        };
        commandList->Begin(beginInfo);
        auto size = m_renderGraph->GetPassSize(m_pass);
        RHI::Viewport viewport{};
        viewport.width = (float)size.width;
        viewport.height = (float)size.height;
        viewport.maxDepth = 1.0f;
        RHI::Scissor scissor{};
        scissor.width = size.width;
        scissor.height = size.height;
        commandList->SetViewport(viewport);
        commandList->SetSicssor(scissor);
        RHI::DrawInfo drawInfo{
            .pipelineState = m_graphicsPipeline,
            .bindGroups = { m_bindGroup, },
            .vertexBuffers = {},
            .indexBuffer = {},
            .parameters = { 3, 1, 0, 0 },
        };
        commandList->Draw(drawInfo);
        commandList->End();

        m_renderGraph->Submit(m_pass, commandList);
    }
};

TL::Ptr<RenderImpl> CreateDeferred()
{
    return TL::CreatePtr<RenderImpl>();
}

class Playground final : public ApplicationBase
{
public:
    Playground()
        : ApplicationBase("Hello, Triangle", 1600, 1200)
        , m_renderer(CreateDeferred())
    {
    }

    void OnInit() override
    {
        m_renderer->Init(*m_window);
    }

    void OnShutdown() override
    {
        m_renderer->Shutdown();
    }

    void OnUpdate(Timestep timestep) override
    {
    }

    void Render() override
    {
        m_renderer->Render();
    }

    void OnEvent(Event& e) override
    {
    }

    TL::Ptr<RPI::Renderer> m_renderer;
};

#include <Examples-Base/Entry.hpp>

int main(int argc, const char* argv[])
{
    using namespace Examples;
    TL::Span args{ argv, (size_t)argc };
    return Entry<Playground>(args);
}
