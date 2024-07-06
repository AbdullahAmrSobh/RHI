#include <Examples-Base/ApplicationBase.hpp>
#include <Examples-Base/SceneGraph.hpp>

#include <RHI/RHI.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include <tracy/Tracy.hpp>

inline static constexpr const char* DEFAULT_SCENE_PATH = "C:/Users/abdul/Desktop/Main.1_Sponza/NewSponza_Main_glTF_002.gltf";

// inline static RHI::Handle<RHI::ImageAttachment> CreateImageAttachment(RHI::RenderGraph& renderGraph, Handle<RHI::Pass> pass, const char* name, RHI::ImageSize2D size, RHI::Format format)
// {
//     RHI::ImageCreateInfo imageCI{};
//     imageCI.name = name;
//     imageCI.format = format;
//     imageCI.usageFlags = RHI::ImageUsage::DepthStencil;
//     imageCI.type = RHI::ImageType::Image2D;
//     imageCI.size = { size.width, size.height, 1 };
//     imageCI.sampleCount = RHI::SampleCount::Samples1;
//     imageCI.mipLevels = 1;
//     imageCI.arrayCount = 1;
//     auto attachment = renderGraph.CreateImage(imageCI);

//     RHI::ImageAttachmentUseInfo attachmentUseInfo{};
//     attachmentUseInfo.usage = RHI::ImageUsage::Color;
//     attachmentUseInfo.loadStoreOperations.loadOperation = RHI::LoadOperation::Discard;
//     attachmentUseInfo.loadStoreOperations.storeOperation = RHI::StoreOperation::Store;
//     attachmentUseInfo.clearValue = { 0.0f, 0.2f, 0.3f, 1.0f };
//     attachmentUseInfo.subresourceRange.arrayCount = 1;
//     attachmentUseInfo.subresourceRange.mipLevelCount = 1;
//     renderGraph.UseImage(pass, attachment, attachmentUseInfo);
//     return attachment;
// }

class BasicRenderer final : public ApplicationBase
{
public:
    BasicRenderer()
        : ApplicationBase("Hello, Triangle", 1600, 1200)
    {
    }

    void OnInit() override
    {
        ZoneScoped;

        m_scene = RHI::CreatePtr<Scene>(m_context.get(), DEFAULT_SCENE_PATH);

        m_renderGraph = m_context->CreateRenderGraph();

        // setup the render graph
        RHI::PassCreateInfo passCreateInfo{};
        passCreateInfo.name = "gBuffer";
        passCreateInfo.queueType = RHI::QueueType::Graphics;
        m_renderPass = m_renderGraph->CreatePass(passCreateInfo);

        auto outputAttachment = m_renderGraph->ImportSwapchain("color-attachment", *m_swapchain);

        RHI::ImageCreateInfo imageCI{};
        imageCI.name = "depth-attachment";
        imageCI.format = RHI::Format::D32;
        imageCI.usageFlags = RHI::ImageUsage::DepthStencil;
        imageCI.type = RHI::ImageType::Image2D;
        imageCI.size = { m_windowWidth, m_windowHeight, 1 };
        imageCI.sampleCount = RHI::SampleCount::Samples1;
        imageCI.mipLevels = 1;
        imageCI.arrayCount = 1;

        auto depthAttachment = m_renderGraph->CreateImage(imageCI);
        imageCI.name = "test-attachment";
        imageCI.format = RHI::Format::RGBA32_FLOAT;
        imageCI.usageFlags = RHI::ImageUsage::Color;
        auto imageAttachment = m_renderGraph->CreateImage(imageCI);

        RHI::ImageAttachmentUseInfo attachmentUseInfo{};
        attachmentUseInfo.usage = RHI::ImageUsage::Color;
        attachmentUseInfo.loadStoreOperations.loadOperation = RHI::LoadOperation::Discard;
        attachmentUseInfo.loadStoreOperations.storeOperation = RHI::StoreOperation::Store;
        attachmentUseInfo.clearValue = { 0.0f, 0.2f, 0.3f, 1.0f };
        attachmentUseInfo.subresourceRange.arrayCount = 1;
        attachmentUseInfo.subresourceRange.mipLevelCount = 1;
        attachmentUseInfo.subresourceRange.imageAspects = RHI::ImageAspect::All;
        m_renderGraph->UseImage(m_renderPass, outputAttachment, attachmentUseInfo);

        attachmentUseInfo.subresourceRange.imageAspects = RHI::ImageAspect::Depth;
        attachmentUseInfo.usage = RHI::ImageUsage::Depth;
        attachmentUseInfo.clearValue.depthStencil.depthValue = 1.0f;
        m_renderGraph->UseImage(m_renderPass, depthAttachment, attachmentUseInfo);

        attachmentUseInfo.subresourceRange.imageAspects = RHI::ImageAspect::Color;
        attachmentUseInfo.usage = RHI::ImageUsage::Color;
        attachmentUseInfo.clearValue = {};
        m_renderGraph->UseImage(m_renderPass, imageAttachment, attachmentUseInfo);

        m_context->CompileRenderGraph(*m_renderGraph);

        m_commandList[0] = m_commandPool[0]->Allocate(RHI::QueueType::Graphics, RHI::CommandListLevel::Primary);
        m_commandList[1] = m_commandPool[1]->Allocate(RHI::QueueType::Graphics, RHI::CommandListLevel::Primary);

        SetupGPUResources(m_scene->m_perDrawData);
        LoadPipeline("./Shaders/Basic.spv");
    }

    void OnShutdown() override
    {
        ZoneScoped;

        m_scene->Shutdown(*m_context);

        m_commandPool[0]->Release(m_commandList[0]);
        m_commandPool[1]->Release(m_commandList[1]);
    }

    void OnUpdate(Timestep timestep) override
    {
        (void)timestep;

        ZoneScoped;

        static float cameraSpeed = 1.0f;

        ImGui::NewFrame();
        ImGui::Text("Basic scene: ");
        ImGui::SliderFloat("camera speed", &cameraSpeed, 0.1f, 5.0f);
        ImGui::Render();

        m_camera.SetMovementSpeed(cameraSpeed);
        m_camera.Update(timestep);

        UpdateUniformBuffers(m_camera.GetView(), m_camera.GetProjection());

        // render code
        RHI::Viewport viewport = {};
        viewport.width = float(m_windowWidth);
        viewport.height = float(m_windowHeight);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        RHI::Scissor scissor = {};
        scissor.width = m_windowWidth;
        scissor.height = m_windowHeight;

        static int i = 0;
        i = i & 1 ? 0 : 1;

        m_commandPool[i]->Reset();
        RHI::CommandListBeginInfo beginInfo{};
        beginInfo.renderGraph = m_renderGraph.get();
        beginInfo.pass = m_renderPass;
        beginInfo.clearValues = {};
        m_commandList[i]->Begin(beginInfo);
        m_commandList[i]->SetViewport(viewport);
        m_commandList[i]->SetSicssor(scissor);
        Draw(*m_commandList[i]);
        m_imguiRenderer->RenderDrawData(ImGui::GetDrawData(), *m_commandList[i]);

        m_commandList[i]->End();
        m_renderGraph->SubmitCommands(m_renderPass, m_commandList[i]);

        m_context->ExecuteRenderGraph(*m_renderGraph);

        (void)m_swapchain->Present();
    }

private:
    void Draw(RHI::CommandList& commandList) const
    {
        ZoneScoped;
        RHI::Handle<RHI::BindGroup> bindGroups = { m_bindGroup };

        RHI::DrawInfo drawInfo{};
        drawInfo.pipelineState = m_graphicsPipeline;
        drawInfo.bindGroups = { bindGroups };

        uint32_t nodeIndex = 0;
        for (const auto& node : m_scene->m_staticSceneNodes)
        {
            for (const auto& handle : node.m_meshes)
            {
                auto mesh = m_scene->m_staticMeshOwner.Get(handle);
                drawInfo.bindGroups = { { m_bindGroup, uint32_t(sizeof(Shader::ObjectTransform) * nodeIndex) } };
                drawInfo.parameters.elementsCount = mesh->elementsCount;
                drawInfo.vertexBuffers = { mesh->position, mesh->normals };
                if (mesh->indcies != RHI::NullHandle)
                {
                    drawInfo.indexBuffer = mesh->indcies;
                }

                commandList.Draw(drawInfo);
            }
            nodeIndex++;
        }
    }

    void UpdateUniformBuffers(glm::mat4 view, glm::mat4 projection)
    {
        m_perFrameData.viewMatrix = view;
        m_perFrameData.projectionMatrix = projection;
        m_perFrameData.viewProjectionMatrix = view * projection;
        m_perFrameData.inverseViewMatrix = glm::inverse(view);

        auto ptr = m_context->MapBuffer(m_perFrameUniformBuffer);
        memcpy(ptr, &m_perFrameData, sizeof(Shader::SceneTransform));
        m_context->UnmapBuffer(m_perFrameUniformBuffer);
    }

    void SetupGPUResources(TL::Span<Shader::ObjectTransform> perDraw)
    {
        m_sampler = m_context->CreateSampler(RHI::SamplerCreateInfo{});

        RHI::BindGroupLayoutCreateInfo bindGroupLayoutInfo{};
        // Per frame uniform buffer
        bindGroupLayoutInfo.bindings[0].access = RHI::Access::Read;
        bindGroupLayoutInfo.bindings[0].stages |= RHI::ShaderStage::Vertex;
        bindGroupLayoutInfo.bindings[0].stages |= RHI::ShaderStage::Pixel;
        bindGroupLayoutInfo.bindings[0].type = RHI::ShaderBindingType::UniformBuffer;
        bindGroupLayoutInfo.bindings[0].arrayCount = 1;

        // per object uniform buffer
        bindGroupLayoutInfo.bindings[1].access = RHI::Access::Read;
        bindGroupLayoutInfo.bindings[1].stages |= RHI::ShaderStage::Vertex;
        bindGroupLayoutInfo.bindings[1].stages |= RHI::ShaderStage::Pixel;
        bindGroupLayoutInfo.bindings[1].type = RHI::ShaderBindingType::DynamicUniformBuffer;
        bindGroupLayoutInfo.bindings[1].arrayCount = 1;

        // TODO: add textures

        m_bindGroupLayout = m_context->CreateBindGroupLayout(bindGroupLayoutInfo);
        m_pipelineLayout = m_context->CreatePipelineLayout({ m_bindGroupLayout });
        m_bindGroup = m_context->CreateBindGroup(m_bindGroupLayout);

        // create buffers

        RHI::BufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.usageFlags = RHI::BufferUsage::Uniform;
        bufferCreateInfo.byteSize = sizeof(Shader::SceneTransform);
        m_perFrameUniformBuffer = m_context->CreateBuffer(bufferCreateInfo).GetValue();
        bufferCreateInfo.byteSize = perDraw.size_bytes();
        m_perObjectUniformBuffer = m_context->CreateBuffer(bufferCreateInfo).GetValue();

        auto ptr = m_context->MapBuffer(m_perObjectUniformBuffer);
        memcpy(ptr, perDraw.data(), perDraw.size() * sizeof(Shader::ObjectTransform));
        m_context->UnmapBuffer(m_perObjectUniformBuffer);

        // update bind groups
        TL::Span<const RHI::ResourceBinding> bindings{
            RHI::ResourceBinding(0, 0, m_perFrameUniformBuffer),
            RHI::ResourceBinding(1, 0, RHI::ResourceBinding::DynamicBufferBinding(m_perObjectUniformBuffer, 0, sizeof(Shader::ObjectTransform))),
        };
        m_context->UpdateBindGroup(m_bindGroup, bindings);
    }

    void LoadPipeline(const char* shaderPath)
    {
        auto shaderCode = ReadBinaryFile(shaderPath);
        auto shaderModule = m_context->CreateShaderModule(shaderCode);
        auto reflectionData = shaderModule->GetReflectionData({ .vsName = "VSMain", .psName = "PSName", .csName = nullptr });

        auto defaultBlendState = RHI::ColorAttachmentBlendStateDesc{
            .blendEnable = false,
            .colorBlendOp = RHI::BlendEquation::Add,
            .srcColor = RHI::BlendFactor::One,
            .dstColor = RHI::BlendFactor::Zero,
            .alphaBlendOp = RHI::BlendEquation::Add,
            .srcAlpha = RHI::BlendFactor::One,
            .dstAlpha = RHI::BlendFactor::Zero,
            .writeMask = RHI::ColorWriteMask::All,
        };
        RHI::GraphicsPipelineCreateInfo createInfo{
            // clang-format off
            .name = "Lighting Pass Pipeline",
            .vertexShaderName = "VSMain",
            .vertexShaderModule = shaderModule.get(),
            .pixelShaderName = "PSMain",
            .pixelShaderModule = shaderModule.get(),
            .layout = m_pipelineLayout,
            .inputAssemblerState = reflectionData.inputAssemblerStateDesc,
            .renderTargetLayout =
                {
                    .colorAttachmentsFormats = { RHI::Format::BGRA8_UNORM, RHI::Format::RGBA32_FLOAT },
                    .depthAttachmentFormat = RHI::Format::D32,
                    .stencilAttachmentFormat = RHI::Format::Unknown,
                },
            .colorBlendState =
                {
                    .blendStates = { defaultBlendState, defaultBlendState },
                    .blendConstants = {}
                },
            .topologyMode = RHI::PipelineTopologyMode::Triangles,
            .rasterizationState =
                {
                    .cullMode = RHI::PipelineRasterizerStateCullMode::BackFace,
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
            // clang-format on
        };
        m_graphicsPipeline = m_context->CreateGraphicsPipeline(createInfo);
    }

private:
    Shader::SceneTransform m_perFrameData = {};
    TL::Vector<Shader::ObjectTransform> m_perDrawData; // todo: make as array

    RHI::Handle<RHI::Sampler> m_sampler;

    RHI::Handle<RHI::Buffer> m_perFrameUniformBuffer;
    RHI::Handle<RHI::Buffer> m_perObjectUniformBuffer;

    RHI::Handle<RHI::BindGroupLayout> m_bindGroupLayout;
    RHI::Handle<RHI::BindGroup> m_bindGroup;

    RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline> m_graphicsPipeline;

    RHI::Ptr<Scene> m_scene;
    RHI::Ptr<RHI::RenderGraph> m_renderGraph;
    RHI::Handle<RHI::Pass> m_renderPass;
    RHI::CommandList* m_commandList[2];
};

RHI_APP_MAIN(BasicRenderer);