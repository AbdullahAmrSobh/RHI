#include <Examples-Base/ApplicationBase.hpp>
#include <ShaderInterface/Core.slang>

#include <RHI/RHI.hpp>

#include <tracy/Tracy.hpp>

#include "Examples-Base/Defer.hpp"

[[maybe_unused]] inline static constexpr RHI::ClearValue BlackColorValue = { .f32{ 0.0f, 0.0f, 0.0f, 0.0f } };
[[maybe_unused]] inline static constexpr RHI::ClearValue WhiteColorValue = { .f32{ 1.0f, 1.0f, 1.0f, 1.0f } };
[[maybe_unused]] inline static constexpr RHI::ClearValue DepthValue = { .depthStencil = { .depthValue = 1.0f, .stencilValue = {} } };

inline static Handle<RHI::ImageAttachment> CreateAttachment(RHI::RenderGraph& renderGraph, Handle<RHI::Pass> pass, const char* name, RHI::Format format, RHI::LoadStoreOperations loadStoreOperations)
{
    RHI::ImageCreateInfo createInfo{
        .name = name,
        .usageFlags = RHI::GetFormatInfo(format).hasRed ? RHI::ImageUsage::Color : RHI::ImageUsage::Depth,
        .type = RHI::ImageType::Image2D,
        .size = { RHI::SizeRelative2D.width, RHI::SizeRelative2D.height, 1 },
        .format = format,
        .sampleCount = RHI::SampleCount::Samples1,
        .mipLevels = 1,
        .arrayCount = 1
    };
    auto attachment = renderGraph.CreateImage(createInfo);
    RHI::ImageViewInfo viewInfo{};
    viewInfo.loadStoreOperations = loadStoreOperations;
    viewInfo.viewType = RHI::ImageViewType::View2D;
    renderGraph.PassUseImage(pass, attachment, viewInfo, (RHI::ImageUsage)((int)createInfo.usageFlags), RHI::ShaderStage::None, RHI::Access::None);
    return attachment;
}

class PassGBuffer
{
public:
    Handle<RHI::Pass> pass = RHI::NullHandle;

    // GBuffer ouptut targets
    Handle<RHI::ImageAttachment> m_albedoTarget = RHI::NullHandle;
    Handle<RHI::ImageAttachment> m_roughnessTarget = RHI::NullHandle;
    Handle<RHI::ImageAttachment> m_wsPositionTarget = RHI::NullHandle;
    Handle<RHI::ImageAttachment> m_normalTarget = RHI::NullHandle;
    Handle<RHI::ImageAttachment> m_depthTarget = RHI::NullHandle;

    Handle<RHI::GraphicsPipeline> m_pipeline = RHI::NullHandle;
    Handle<RHI::PipelineLayout> m_pipelineLayout = RHI::NullHandle;
    Handle<RHI::BindGroup> m_bindGroup = RHI::NullHandle;

    Handle<RHI::Buffer> m_sceneTransform, m_sceneObjectsTransforms;

    Handle<RHI::CommandList> cmdDrawStaticMeshes = RHI::NullHandle;

    void Init(RHI::Context& context)
    {
        Handle<RHI::BindGroupLayout> gBufferBGL;
        {
            RHI::BindGroupLayoutCreateInfo createInfo{
                /* scene transform   */ RHI::ShaderBinding{ .type = RHI::ShaderBindingType::UniformBuffer, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
                /* objects transform */ RHI::ShaderBinding{ .type = RHI::ShaderBindingType::DynamicUniformBuffer, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
            };
            gBufferBGL = context.CreateBindGroupLayout(createInfo);
        }
        re_defer
        {
            context.DestroyBindGroupLayout(gBufferBGL);
        };

        m_bindGroup = context.CreateBindGroup({ gBufferBGL });

        RHI::ResourceBinding bindings[] = {
            RHI::ResourceBinding(0, 0, m_sceneTransform),
            RHI::ResourceBinding(1, 0, RHI::ResourceBinding::DynamicBufferBinding{ m_sceneObjectsTransforms, 0, sizeof(glm::mat4) }),
        };
        context.UpdateBindGroup(m_bindGroup, bindings);

        {
            RHI::PipelineLayoutCreateInfo createInfo{};
            createInfo.name = "Lighting Pass Pipeline Layout";
            // createInfo.layouts[0] = lightingPassInputAttachmentsBGL;
            m_pipelineLayout = context.CreatePipelineLayout(createInfo);
        }

        {
            auto shaderCode = ReadBinaryFile("LightingPass.spv");
            auto shaderModule = context.CreateShaderModule(shaderCode);
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
                .inputAssemblerState = {},
                .renderTargetLayout =
                    {
                        .colorAttachmentsFormats = { RHI::Format::BGRA8_UNORM },
                        .depthAttachmentFormat = RHI::Format::Unknown,
                        .stencilAttachmentFormat = RHI::Format::Unknown,
                    },
                .colorBlendState =
                    {
                        .blendStates = { defaultBlendState, defaultBlendState, defaultBlendState, defaultBlendState},
                        .blendConstants = {}
                    },
                .topologyMode = RHI::PipelineTopologyMode::Triangles,
                .rasterizationState =
                    {
                        .cullMode = RHI::PipelineRasterizerStateCullMode::BackFace,
                        .fillMode = RHI::PipelineRasterizerStateFillMode::Triangle,
                        .frontFace = RHI::PipelineRasterizerStateFrontFace::Clockwise,
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
            m_pipeline = context.CreateGraphicsPipeline(createInfo);
        }
    }

    void Shutdown(RHI::Context& context)
    {
        context.DestroyGraphicsPipeline(m_pipeline);
        context.DestroyPipelineLayout(m_pipelineLayout);
        context.DestroyBindGroup(m_bindGroup);
        context.DestroyBuffer(m_sceneTransform);
        context.DestroyBuffer(m_sceneObjectsTransforms);
    }

    void BuildCommandLists(RHI::CommandEncoder& commandEncoder, Handle<RHI::CommandList> commandList, [[maybe_unused]] const Scene& scene)
    {
        // if (scene.isSceneValid == false)
        //     return;

        RHI::CommandListBeginInfo beginInfo{};

        beginInfo.pass = this->pass;
        commandEncoder.Begin(commandList, beginInfo);

        // for (auto node : scene.m_graph)
        // {
        //     for (auto mesh : node.m_meshes)
        //     {
        //         RHI::DrawInfo drawInfo
        //         {
        //             .vertexBuffers = { RHI::BufferBindingInfo{  .buffer = mesh } }
        //         };
        //         commandEncoder.Draw(commandList, drawInfo);
        //     }
        // }

        commandEncoder.End(commandList);
    }

    void SetupAttachments(RHI::RenderGraph& renderGraph)
    {
        // clang-format off
        ZoneScoped;

        this->pass = renderGraph.CreatePass({ .name = "GBuffer", .flags = RHI::PassFlags::Graphics });

        RHI::LoadStoreOperations loadStoreOperations
        {
            .clearValue = BlackColorValue,
            .loadOperation = RHI::LoadOperation::Discard,
            .storeOperation = RHI::StoreOperation::Store,
        };

        this->m_albedoTarget     = CreateAttachment(renderGraph, pass, "albedoTarget",     RHI::Format::RGBA8_UNORM, loadStoreOperations);
        this->m_roughnessTarget  = CreateAttachment(renderGraph, pass, "roughnessTarget",  RHI::Format::RGBA8_UNORM, loadStoreOperations);
        this->m_wsPositionTarget = CreateAttachment(renderGraph, pass, "wsPositionTarget", RHI::Format::RGBA8_UNORM, loadStoreOperations);
        this->m_normalTarget     = CreateAttachment(renderGraph, pass, "normalTarget",     RHI::Format::RGBA8_UNORM, loadStoreOperations);

        loadStoreOperations.clearValue = DepthValue;
        this->m_depthTarget = CreateAttachment(renderGraph, pass, "depthTarget", RHI::Format::RGBA8_UNORM, loadStoreOperations);
    }

    void UpdateBindGroup([[maybe_unused]] RHI::Context& context, [[maybe_unused]] RHI::RenderGraph& renderGraph, [[maybe_unused]] const Scene& scene)
    {

    }

    void Execute(RHI::RenderGraph& renderGraph, RHI::CommandEncoder& commandEncoder, const Scene& scene)
    {
        ZoneScoped;

        BuildCommandLists(commandEncoder, this->cmdDrawStaticMeshes, scene);

        Handle<RHI::CommandList> commandList;
        commandEncoder.Allocate(RHI::CommandFlags::Transient, commandList);

        RHI::CommandListBeginInfo beginInfo
        {
            .renderGraph = &renderGraph,
            .pass = this->pass,
            .loadStoreOperations = {}
        };
        commandEncoder.Begin(commandList, beginInfo);
        commandEncoder.SetScissor(commandList, scene.m_scissor);
        commandEncoder.SetViewport(commandList, scene.m_viewport);
        commandEncoder.Execute(commandList, this->cmdDrawStaticMeshes);
        commandEncoder.End(commandList);

        renderGraph.Submit(this->pass, commandEncoder, commandList);
    }
};

class PassLighting
{
public:
    Handle<RHI::Pass>             m_pass;

    Handle<RHI::ImageAttachment>  m_colorAttachment;
    Handle<RHI::ImageAttachment>  m_depthAttachment;

    Handle<RHI::GraphicsPipeline> m_pipeline;
    Handle<RHI::PipelineLayout>   m_pipelineLayout;
    Handle<RHI::BindGroup>        m_bindGroup;

    Handle<RHI::CommandList>      m_commandList;

    void Init(RHI::Context& context, Handle<RHI::ImageAttachment> colorAttachment)
    {
        ZoneScoped;

        auto shaderCode = ReadBinaryFile("LightingPass.spv");
        auto shaderModule = context.CreateShaderModule(shaderCode);

        m_colorAttachment = colorAttachment;

        Handle<RHI::BindGroupLayout> lightingPassInputAttachmentsBGL;
        {
            RHI::BindGroupLayoutCreateInfo createInfo
            {
               /* albedoTarget      */ RHI::ShaderBinding{ .type = RHI::ShaderBindingType::InputAttachment , .access =RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
               /* roughnessTarget   */ RHI::ShaderBinding{ .type = RHI::ShaderBindingType::InputAttachment , .access =RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
               /* wsPositionTarget  */ RHI::ShaderBinding{ .type = RHI::ShaderBindingType::InputAttachment , .access =RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
               /* normalTarget      */ RHI::ShaderBinding{ .type = RHI::ShaderBindingType::InputAttachment , .access =RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
               /* depthTarget       */ RHI::ShaderBinding{ .type = RHI::ShaderBindingType::InputAttachment , .access =RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
            };
            lightingPassInputAttachmentsBGL = context.CreateBindGroupLayout(createInfo);
        }

        {
            RHI::PipelineLayoutCreateInfo createInfo {};
            createInfo.name = "Lighting Pass Pipeline Layout";
            createInfo.layouts[0] = lightingPassInputAttachmentsBGL;
            m_pipelineLayout = context.CreatePipelineLayout(createInfo);
        }

        {
            RHI::GraphicsPipelineCreateInfo createInfo
            {
                // clang-format off
                .name = "Lighting Pass Pipeline",
                .vertexShaderName = "VSMain",
                .vertexShaderModule = shaderModule.get(),
                .pixelShaderName = "PSMain",
                .pixelShaderModule = shaderModule.get(),
                .layout = m_pipelineLayout,
                .inputAssemblerState = {},
                .renderTargetLayout =
                    {
                        .colorAttachmentsFormats = { RHI::Format::BGRA8_UNORM },
                        .depthAttachmentFormat = RHI::Format::Unknown,
                        .stencilAttachmentFormat = RHI::Format::Unknown,
                    },
                .colorBlendState =
                    {
                        .blendStates =
                            {{
                                .blendEnable = false,
                                .colorBlendOp = RHI::BlendEquation::Add,
                                .srcColor = RHI::BlendFactor::One,
                                .dstColor = RHI::BlendFactor::Zero,
                                .alphaBlendOp = RHI::BlendEquation::Add,
                                .srcAlpha = RHI::BlendFactor::One,
                                .dstAlpha = RHI::BlendFactor::Zero,
                                .writeMask = RHI::ColorWriteMask::All,
                            }},
                        .blendConstants = {}
                    },
                .topologyMode = RHI::PipelineTopologyMode::Triangles,
                .rasterizationState =
                    {
                        .cullMode = RHI::PipelineRasterizerStateCullMode::BackFace,
                        .fillMode = RHI::PipelineRasterizerStateFillMode::Triangle,
                        .frontFace = RHI::PipelineRasterizerStateFrontFace::Clockwise,
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
            m_pipeline = context.CreateGraphicsPipeline(createInfo);
        }

        m_bindGroup = context.CreateBindGroup(lightingPassInputAttachmentsBGL);
    }

    void Shutdown(RHI::Context& context)
    {
        ZoneScoped;

        context.DestroyBindGroup(m_bindGroup);
        context.DestroyPipelineLayout(m_pipelineLayout);
        context.DestroyGraphicsPipeline(m_pipeline);
    }

    void SetupAttachments(RHI::RenderGraph& renderGraph, const PassGBuffer& passGBuffer)
    {
        ZoneScoped;

        // shader input
        renderGraph.PassUseImage(m_pass, passGBuffer.m_albedoTarget, RHI::ImageUsage::Sampled, RHI::ShaderStage::Pixel, RHI::Access::Read);
        renderGraph.PassUseImage(m_pass, passGBuffer.m_roughnessTarget, RHI::ImageUsage::Sampled, RHI::ShaderStage::Pixel, RHI::Access::Read);
        renderGraph.PassUseImage(m_pass, passGBuffer.m_wsPositionTarget, RHI::ImageUsage::Sampled, RHI::ShaderStage::Pixel, RHI::Access::Read);
        renderGraph.PassUseImage(m_pass, passGBuffer.m_normalTarget, RHI::ImageUsage::Sampled, RHI::ShaderStage::Pixel, RHI::Access::Read);
        renderGraph.PassUseImage(m_pass, passGBuffer.m_depthTarget, RHI::ImageUsage::Sampled, RHI::ShaderStage::Pixel, RHI::Access::Read);

        // Output color and depth attachments
        renderGraph.PassUseImage(m_pass, m_colorAttachment, RHI::ImageUsage::Color, RHI::ShaderStage::None, RHI::Access::None);
        renderGraph.PassUseImage(m_pass, m_depthAttachment, RHI::ImageUsage::Depth, RHI::ShaderStage::None, RHI::Access::None);
    }

    void UpdateBindGroup(RHI::Context& context, RHI::RenderGraph& renderGraph, const PassGBuffer& gBuffer)
    {
        RHI::ResourceBinding bindings[] = {
            RHI::ResourceBinding(0, 0, renderGraph.PassGetImageView(this->m_pass, gBuffer.m_albedoTarget)),
            RHI::ResourceBinding(1, 0, renderGraph.PassGetImageView(this->m_pass, gBuffer.m_roughnessTarget)),
            RHI::ResourceBinding(2, 0, renderGraph.PassGetImageView(this->m_pass, gBuffer.m_wsPositionTarget)),
            RHI::ResourceBinding(3, 0, renderGraph.PassGetImageView(this->m_pass, gBuffer.m_normalTarget)),
            RHI::ResourceBinding(4, 0, renderGraph.PassGetImageView(this->m_pass, gBuffer.m_depthTarget)),
        };
        context.UpdateBindGroup(m_bindGroup, bindings);
    }

    void Execute(RHI::RenderGraph& renderGraph, RHI::CommandEncoder& encoder, const Scene& sceneData)
    {
        ZoneScoped;

        RHI::CommandListBeginInfo beginInfo{
            .renderGraph = &renderGraph,
            .pass = m_pass,
            .loadStoreOperations = {}
        };
        encoder.Begin(m_commandList, beginInfo);
        encoder.SetScissor(m_commandList, sceneData.m_scissor);
        encoder.SetViewport(m_commandList, sceneData.m_viewport);
        RHI::DrawInfo drawInfo{
            .pipelineState = m_pipeline,
            .bindGroups = { { .bindGroup = m_bindGroup, .dynamicOffsets = {} } },
            .vertexBuffers = {},
            .indexBuffer = {},
            .parameters = {
                .elementsCount = 6,
                .instanceCount = 1,
                .firstElement = 0,
                .vertexOffset = 0,
                .firstInstance = 0,
            }
        };
        encoder.Draw(m_commandList, drawInfo);
        encoder.End(m_commandList);
        renderGraph.Submit(m_pass, encoder, m_commandList);
    }
};

// clang-format on

class DeferredRenderer final : public ApplicationBase
{
public:
    Ptr<RHI::RenderGraph> m_renderGraph;
    Ptr<RHI::CommandEncoder> m_commandEncoder;

    PassGBuffer m_passGBuffer;
    PassLighting m_passLighting;

    DeferredRenderer()
        : ApplicationBase("Basic Deferred PBR Renderer", 1600, 1200)
    {
    }

    void OnInit() override
    {
        ZoneScoped;

        {
            ZoneScopedN("RenderGraph Passes Initialization");

            auto colorAttachment = m_renderGraph->ImportSwapchain("Output", *m_swapchain);
            m_passGBuffer.Init(*this->m_context);
            m_passLighting.Init(*this->m_context, colorAttachment);
        }

        {
            ZoneScopedN("RenderGraph Attachments Setup");
            m_renderGraph = m_context->CreateRenderGraph();
            m_passGBuffer.SetupAttachments(*m_renderGraph);
            m_passLighting.SetupAttachments(*m_renderGraph, m_passGBuffer);
            m_context->CompileRenderGraph(*m_renderGraph);
        }

        {
            ZoneScopedN("RenderGraph Passes bind groups setup");
            m_passGBuffer.UpdateBindGroup(*m_context, *m_renderGraph, *m_scene);
            m_passLighting.UpdateBindGroup(*m_context, *m_renderGraph, m_passGBuffer);
        }
    }

    void OnShutdown() override
    {
        ZoneScoped;

        m_passLighting.Shutdown(*this->m_context);
        m_passGBuffer.Shutdown(*this->m_context);
    }

    void OnUpdate(Timestep timestep) override
    {
        ZoneScoped;

        static float cameraSpeed = 1.0f;

        // ImGui::NewFrame();
        // ImGui::Text("Basic scene");
        // ImGui::SliderFloat("Camera SPeed", &cameraSpeed, 0.1f, 10.0f);
        // ImGui::Render();

        m_camera.SetMovementSpeed(cameraSpeed);

        SceneRunSimulation(timestep);
        SceneRender();

        auto result = m_swapchain->Present();
        if (result == RHI::ResultCode::SwapchainOutOfDate || result == RHI::ResultCode::SwapchainSurfaceLost)
        {
            result = m_swapchain->Recreate({ m_window->GetWidth(), m_window->GetHeight() });
            RHI_ASSERT(RHI::IsSucess(result));
        }
    }

    void SceneRunSimulation(Timestep timestep)
    {
        m_camera.Update(timestep);
    }

    void SceneRender()
    {
        ZoneScoped;

        m_passGBuffer.Execute(*m_renderGraph, *m_commandEncoder, *m_scene);
        m_passLighting.Execute(*m_renderGraph, *m_commandEncoder, *m_scene);
        m_context->ExecuteRenderGraph(*m_renderGraph);
    }
};

#include "Examples-Base/Entry.hpp"
RHI_APP_MAIN(DeferredRenderer);