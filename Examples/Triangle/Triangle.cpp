#include "Camera.hpp"

#include <Assets/Importer.hpp>
#include <Examples-Base/ApplicationBase.hpp>
#include <RPI/ConstantBuffer.hpp>
#include <RPI/Renderer.hpp>
#include <RPI/View.hpp>
#include <tracy/Tracy.hpp>

#include <TL/FileSystem/FileSystem.hpp>
#include <TL/Allocator/MemPlumber.hpp>

#include <ShaderInterface/Core.slang>

using namespace Examples;

class RenderImpl final : public RPI::Renderer
{
public:
    RPI::ConstantBuffer<SI::ViewCB> m_viewCB;

    RHI::Handle<RHI::BindGroup> m_bindGroup;

    RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline> m_graphicsPipeline;
    RHI::Handle<RHI::Pass> m_pass;

    // RHI::Handle<RHI::Buffer> m_indexBuffer, m_vertexBuffer;

    // void InitIndexAndVertex()
    // {
    //     std::fstream file {"C:/Users/abdul/Desktop/Main.1_Sponza/cache/meshes/arch_stones_01-1.fgmesh", std::ios::binary };
    //     Assets::Mesh mesh {};
    //     TL::BinaryArchive archive {file};
    //     archive.Decode(mesh);
    // }

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
        auto shaderModule = context->CreateShaderModule(spv);

        m_viewCB.Init(*m_context);
        m_viewCB->color = { 0.4, 1.0, 0.4, 1.0 };
        m_viewCB.Update();

        RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{};
        bindGroupLayoutCI.bindings[0].type = RHI::BindingType::UniformBuffer;
        bindGroupLayoutCI.bindings[0].access = RHI::Access::Read;
        bindGroupLayoutCI.bindings[0].arrayCount = 1;
        bindGroupLayoutCI.bindings[0].stages = RHI::ShaderStage::Pixel;
        bindGroupLayoutCI.bindings[0].stages |= RHI::ShaderStage::Vertex;
        auto bindGroupLayout = context->CreateBindGroupLayout(bindGroupLayoutCI);

        m_bindGroup = context->CreateBindGroup(bindGroupLayout);
        context->UpdateBindGroup(m_bindGroup, { RHI::BindGroupUpdateInfo(0, 0, m_viewCB.GetBuffer()) });

        RHI::PipelineLayoutCreateInfo pipelineLayoutCI{ bindGroupLayout };
        m_pipelineLayout = context->CreatePipelineLayout(pipelineLayoutCI);

        context->DestroyBindGroupLayout(bindGroupLayout);

        RHI::GraphicsPipelineCreateInfo pipelineCI{};
        pipelineCI.name = "Basic";
        pipelineCI.vertexShaderName = "VSMain";
        pipelineCI.vertexShaderModule = shaderModule.get();
        pipelineCI.pixelShaderName = "PSMain";
        pipelineCI.pixelShaderModule = shaderModule.get();
        pipelineCI.layout = m_pipelineLayout;
        pipelineCI.inputAssemblerState = {};
        pipelineCI.renderTargetLayout.colorAttachmentsFormats[0] = RHI::Format::RGBA8_UNORM;
        pipelineCI.colorBlendState.blendStates[0].blendEnable = true;
        pipelineCI.colorBlendState.blendStates[0].colorBlendOp = RHI::BlendEquation::Add;
        pipelineCI.colorBlendState.blendStates[0].srcColor = RHI::BlendFactor::SrcAlpha;
        pipelineCI.colorBlendState.blendStates[0].dstColor = RHI::BlendFactor::OneMinusSrcAlpha;
        pipelineCI.colorBlendState.blendStates[0].alphaBlendOp = RHI::BlendEquation::Add;
        pipelineCI.colorBlendState.blendStates[0].srcAlpha = RHI::BlendFactor::One;
        pipelineCI.colorBlendState.blendStates[0].dstAlpha = RHI::BlendFactor::Zero;
        pipelineCI.colorBlendState.blendStates[0].writeMask = RHI::ColorWriteMask::All;
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
        m_graphicsPipeline = context->CreateGraphicsPipeline(pipelineCI);

        return RPI::ResultCode::Sucess;
    }

    void OnShutdown() override
    {
        m_viewCB.Shutdown(*m_context);
        m_context->DestroyBindGroup(m_bindGroup);
        m_context->DestroyPipelineLayout(m_pipelineLayout);
        m_context->DestroyGraphicsPipeline(m_graphicsPipeline);
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
            .bindGroups = {
                           m_bindGroup,
                           },
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
        m_camera.m_window = m_window.get();
        m_camera.SetPerspective(60.0f, 1600.0f / 1200.0f, 0.1f, 10000.0f);
        m_camera.SetRotationSpeed(0.0002f);
        m_camera.SetPosition({});
        glm::vec3 rotation{};
        rotation = {};
        m_camera.SetRotation(rotation);

        m_renderer->Init(*m_window);
    }

    void OnShutdown() override
    {
        m_renderer->Shutdown();
    }

    void OnUpdate(Timestep timestep) override
    {
        // Accumulate the elapsed time based on the fixed timestep
        static float accumulatedTime = 0.0f;
        accumulatedTime += (float)timestep.Seconds(); // Assuming GetSeconds() returns the timestep in seconds

        constexpr float frequency = 2.0f;  // Controls the speed of the transition
        constexpr float amplitude = 10.5f; // Amplitude for color variation
        constexpr float offset = 0.5f;     // Offset to ensure values are in the range [0, 1]

        // Calculate the color components with larger phase shifts for more distinct colors
        m_renderer->m_viewCB->color.r = glm::sin((glm::sin(frequency * accumulatedTime) * amplitude + offset));
        m_renderer->m_viewCB->color.g = glm::sin((glm::sin(frequency * accumulatedTime + glm::pi<float>()) * amplitude + offset));
        m_renderer->m_viewCB->color.b = glm::sin((glm::sin(frequency * accumulatedTime + 2.0f * glm::pi<float>()) * amplitude + offset));

        m_camera.Update(timestep);
        TL_LOG_INFO("({}, {}, {}, {})", m_renderer->m_viewCB->color.r, m_renderer->m_viewCB->color.g, m_renderer->m_viewCB->color.b, m_renderer->m_viewCB->color.a);

        m_camera.Update(timestep);
    }

    void Render() override
    {
        m_renderer->m_viewCB->worldToClipMatrix = m_camera.GetProjection() * m_camera.GetView();
        m_renderer->m_viewCB->worldToViewMatrix = m_camera.GetView();
        m_renderer->m_viewCB->viewToClipMatrix = m_camera.GetProjection();
        m_renderer->m_viewCB.Update();

        m_renderer->Render();
    }

    void OnEvent(Event& e) override
    {
    }

    Camera m_camera;

    TL::Ptr<RenderImpl> m_renderer;
};

#include <Examples-Base/Entry.hpp>

int main(int argc, const char* argv[])
{
    TL::MemPlumber::start();
    using namespace Examples;
    TL::Span args{ argv, (size_t)argc };
    auto result = Entry<Playground>(args);
    size_t memLeakCount, memLeakSize;
    TL::MemPlumber::memLeakCheck(memLeakCount, memLeakSize);
    return result;
}
