#include "Examples-Base/Renderer.hpp"
#include "Examples-Base/Scene.hpp"
#include <Examples-Base/FileSystem.hpp>
#include <Examples-Base/Window.hpp>
#include "Examples-Base/Log.hpp"

#include "ShaderInterface/Core.slang"

#include <RHI/RHI.hpp>

namespace Examples
{
    inline static Handle<RHI::ImageAttachment> CreateTarget(RHI::RenderGraph& renderGraph, Handle<RHI::Pass> pass, const char* name, RHI::Format format, Flags<RHI::ImageUsage> usage)
    {
        bool isDepth = format == RHI::Format::D32;

        RHI::ImageCreateInfo imageCI{};
        imageCI.name = name;
        imageCI.usageFlags = usage;
        imageCI.type = RHI::ImageType::Image2D;
        imageCI.size = {};
        imageCI.format = format;
        imageCI.sampleCount = RHI::SampleCount::Samples1;
        imageCI.mipLevels = 1;
        imageCI.arrayCount = 1;
        auto imageAttachment = renderGraph.CreateImage(imageCI);

        RHI::ImageViewInfo viewInfo{};
        viewInfo.type = RHI::ImageViewType::View2D;
        viewInfo.subresources.imageAspects = RHI::GetFormatAspects(format);
        viewInfo.subresources.arrayCount = 1;
        viewInfo.subresources.mipLevelCount = 1;
        renderGraph.PassUseImage(pass, imageAttachment, viewInfo, isDepth ? RHI::ImageUsage::Depth : RHI::ImageUsage::Color, RHI::ShaderStage::None, RHI::Access::None);

        return imageAttachment;
    }

    struct PassGBuffer
    {
        Handle<RHI::PipelineLayout> m_pipelineLayout;
        Handle<RHI::GraphicsPipeline> m_pipeline;
        Handle<RHI::BindGroup> m_bindGroup;
        Handle<RHI::Sampler> m_sampler;

        Handle<RHI::Buffer> m_sceneTransform;
        Handle<RHI::Buffer> m_objectsTransform;

        Handle<RHI::Pass> m_pass;
        Handle<RHI::ImageAttachment> m_colorAttachment;
        Handle<RHI::ImageAttachment> m_normalAttachment;
        Handle<RHI::ImageAttachment> m_depthAttachment;

        ResultCode Init(RHI::Context& context)
        {
            {
                RHI::SamplerCreateInfo samplerCI{};
                samplerCI.name = "ImGui-Sampler";
                samplerCI.filterMin = RHI::SamplerFilter::Linear;
                samplerCI.filterMag = RHI::SamplerFilter::Linear;
                samplerCI.filterMip = RHI::SamplerFilter::Linear;
                samplerCI.compare = RHI::SamplerCompareOperation::Always;
                samplerCI.mipLodBias = 0.0f;
                samplerCI.addressU = RHI::SamplerAddressMode::Repeat;
                samplerCI.addressV = RHI::SamplerAddressMode::Repeat;
                samplerCI.addressW = RHI::SamplerAddressMode::Repeat;
                samplerCI.minLod = 0.0f;
                samplerCI.maxLod = 1.0f;
                m_sampler = context.CreateSampler(samplerCI);
            }

            auto shaderModuleCode = ReadBinaryFile("Shaders/Basic.spv");

            auto shaderMoudule = context.CreateShaderModule(shaderModuleCode);

            RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{};
            // Per frame uniform buffer
            bindGroupLayoutCI.bindings[0].access = RHI::Access::Read;
            bindGroupLayoutCI.bindings[0].stages |= RHI::ShaderStage::Vertex;
            bindGroupLayoutCI.bindings[0].stages |= RHI::ShaderStage::Pixel;
            bindGroupLayoutCI.bindings[0].type = RHI::BindingType::UniformBuffer;
            bindGroupLayoutCI.bindings[0].arrayCount = 1;

            // per object uniform buffer
            bindGroupLayoutCI.bindings[1].access = RHI::Access::Read;
            bindGroupLayoutCI.bindings[1].stages |= RHI::ShaderStage::Vertex;
            bindGroupLayoutCI.bindings[1].stages |= RHI::ShaderStage::Pixel;
            bindGroupLayoutCI.bindings[1].type = RHI::BindingType::DynamicUniformBuffer;
            bindGroupLayoutCI.bindings[1].arrayCount = 1;

            // create dummy sampler
            bindGroupLayoutCI.bindings[2].access = RHI::Access::Read;
            bindGroupLayoutCI.bindings[2].stages |= RHI::ShaderStage::Pixel;
            bindGroupLayoutCI.bindings[2].type = RHI::BindingType::Sampler;
            bindGroupLayoutCI.bindings[2].arrayCount = 1;

            // bindless textures
            bindGroupLayoutCI.bindings[3].access = RHI::Access::Read;
            bindGroupLayoutCI.bindings[3].stages |= RHI::ShaderStage::Pixel;
            bindGroupLayoutCI.bindings[3].type = RHI::BindingType::SampledImage;
            bindGroupLayoutCI.bindings[3].arrayCount = RHI::ShaderBinding::VariableArraySize;

            auto bindGroupLayout = context.CreateBindGroupLayout(bindGroupLayoutCI);
            m_bindGroup = context.CreateBindGroup(bindGroupLayout, 49);

            RHI::PipelineLayoutCreateInfo pipelineLayoutCI{ bindGroupLayout };
            m_pipelineLayout = context.CreatePipelineLayout(pipelineLayoutCI);

            context.DestroyBindGroupLayout(bindGroupLayout);

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

            // clang-format off
            RHI::GraphicsPipelineCreateInfo pipelineCI
            {
                .name = "PBR Pipeline",
                .vertexShaderName = "VSMain",
                .vertexShaderModule = shaderMoudule.get(),
                .pixelShaderName = "PSMain",
                .pixelShaderModule = shaderMoudule.get(),
                .layout = m_pipelineLayout,
                .inputAssemblerState = shaderMoudule->GetReflectionData({ .vsName = "VSMain", .psName = "PSName", .csName = nullptr }).inputAssemblerStateDesc,
                .renderTargetLayout =
                    {
                        .colorAttachmentsFormats = { RHI::Format::RGBA8_UNORM, RHI::Format::RGBA16_UNORM },
                        .depthAttachmentFormat = RHI::Format::D32,
                        .stencilAttachmentFormat = RHI::Format::Unknown,
                    },
                .colorBlendState =
                    {
                        .blendStates =
                        {
                            defaultBlendState,
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
            // clang-format on
            m_pipeline = context.CreateGraphicsPipeline(pipelineCI);

            return ResultCode::Success;
        }

        void Shutdown(RHI::Context& context)
        {
            context.DestroyGraphicsPipeline(m_pipeline);
            context.DestroyPipelineLayout(m_pipelineLayout);
        }

        ResultCode Setup(RHI::RenderGraph& renderGraph)
        {
            RHI::PassCreateInfo passCI{};
            passCI.name = "Pass-GBuffer";
            passCI.flags = RHI::PassFlags::Graphics;
            m_pass = renderGraph.CreatePass(passCI);

            m_colorAttachment = CreateTarget(renderGraph, m_pass, "GBuffer-Color", RHI::Format::RGBA8_UNORM, RHI::ImageUsage::Color | RHI::ImageUsage::ShaderResource);
            m_normalAttachment = CreateTarget(renderGraph, m_pass, "GBuffer-Normal", RHI::Format::RGBA16_UNORM, RHI::ImageUsage::Color | RHI::ImageUsage::ShaderResource);
            m_depthAttachment = CreateTarget(renderGraph, m_pass, "GBuffer-Depth", RHI::Format::D32, RHI::ImageUsage::Depth | RHI::ImageUsage::ShaderResource);

            return ResultCode::Success;
        }

        void Execute(RHI::RenderGraph& renderGraph, RHI::CommandPool& commandPool, const Scene& scene)
        {
            static bool init = false;
            if (!init)
            {
                init = true;

                RHI::BufferCreateInfo bufferCreateInfo{};
                bufferCreateInfo.usageFlags = RHI::BufferUsage::Uniform;
                bufferCreateInfo.byteSize = sizeof(SceneTransform);
                m_sceneTransform = renderGraph.m_context->CreateBuffer(bufferCreateInfo).GetValue();
                bufferCreateInfo.byteSize = scene.m_meshesTransform.size() * sizeof(ObjectTransform);
                m_objectsTransform = renderGraph.m_context->CreateBuffer(bufferCreateInfo).GetValue();

                // update objects transofmr
                auto ptr = renderGraph.m_context->MapBuffer(m_objectsTransform);
                memcpy(ptr, scene.m_meshesTransform.data(), scene.m_meshesTransform.size() * sizeof(ObjectTransform));
                renderGraph.m_context->UnmapBuffer(m_objectsTransform);

                // update bind groups
                TL::Span<const RHI::BindGroupUpdateInfo> bindings{
                    RHI::BindGroupUpdateInfo(0, 0, m_sceneTransform),
                    RHI::BindGroupUpdateInfo(1, 0, RHI::BindGroupUpdateInfo::DynamicBufferBinding(m_objectsTransform, 0, sizeof(ObjectTransform))),
                    RHI::BindGroupUpdateInfo(2, 0, m_sampler),
                    RHI::BindGroupUpdateInfo(3, 0, TL::Span{ scene.imagesViews.data(), scene.imagesViews.size() }),
                };
                renderGraph.m_context->UpdateBindGroup(m_bindGroup, bindings);
            }

            SceneTransform sceneTransform{};
            sceneTransform.viewMatrix = scene.m_viewMatrix;
            sceneTransform.projectionMatrix = scene.m_projectionMatrix;
            sceneTransform.viewProjectionMatrix = scene.m_viewMatrix * scene.m_projectionMatrix;
            sceneTransform.inverseViewMatrix = glm::inverse(scene.m_viewMatrix);
            auto ptr = renderGraph.m_context->MapBuffer(m_sceneTransform);
            memcpy(ptr, &sceneTransform, sizeof(SceneTransform));
            renderGraph.m_context->UnmapBuffer(m_sceneTransform);

            auto commandList = commandPool.Allocate(RHI::QueueType::Graphics, RHI::CommandListLevel::Primary);

            auto size = renderGraph.GetPassSize(m_pass);
            RHI::Viewport viewport{};
            viewport.width = (float)size.width;
            viewport.height = (float)size.height;
            viewport.maxDepth = 1.0f;
            RHI::Scissor scissor{};
            scissor.width = size.width;
            scissor.height = size.height;

            RHI::CommandListBeginInfo beginInfo{};
            beginInfo.renderGraph = &renderGraph;
            beginInfo.pass = m_pass;
            beginInfo.loadStoreOperations = {
                { .clearValue = { .f32 = { 0.4f, 0.3f, 0.1f, 1.0f } }, .loadOperation = RHI::LoadOperation::Discard, .storeOperation = RHI::StoreOperation::Store },
                { .clearValue = { .f32 = { 0.3f, 0.4f, 0.2f, 1.0f } }, .loadOperation = RHI::LoadOperation::Discard, .storeOperation = RHI::StoreOperation::Store },
                { .clearValue = { .depthStencil = { 1.0f, 0 } },       .loadOperation = RHI::LoadOperation::Discard, .storeOperation = RHI::StoreOperation::Store },
            };

            commandList->Begin(beginInfo);
            commandList->SetViewport(viewport);
            commandList->SetSicssor(scissor);

            for (uint32_t i = 0; i < scene.m_meshesStatic.size(); i++)
            {
                auto mesh = scene.m_meshes[i];

                TL::Vector<RHI::BufferBindingInfo> bindingInfo{};

                RHI::DrawInfo drawInfo{};
                drawInfo.parameters.instanceCount = 1;
                drawInfo.parameters.elementsCount = mesh->elementsCount;
                drawInfo.pipelineState = m_pipeline;

                drawInfo.indexBuffer.buffer = mesh->m_index;
                bindingInfo.push_back({ mesh->m_position, 0 });
                bindingInfo.push_back({ mesh->m_normal, 0 });
                bindingInfo.push_back({ mesh->m_texCoord, 0 });

                drawInfo.vertexBuffers = bindingInfo;

                drawInfo.bindGroups = RHI::BindGroupBindingInfo{ m_bindGroup, i * sizeof(ObjectTransform) };
                commandList->Draw(drawInfo);
            }

            commandList->End();
            renderGraph.Submit(m_pass, commandList);
        }
    };

    struct PassLighting
    {
        Handle<RHI::BindGroup> m_bindGroup;
        Handle<RHI::GraphicsPipeline> m_pipeline;
        Handle<RHI::PipelineLayout> m_pipelineLayout;

        Handle<RHI::Pass> m_pass;
        Handle<RHI::ImageAttachment> m_outputAttachment;

        ResultCode Init(RHI::Context& context)
        {
            auto shaderModuleCode = ReadBinaryFile("Shaders/Compose.spv");
            auto shaderMoudule = context.CreateShaderModule(shaderModuleCode);

            RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{
                RHI::ShaderBinding{ .type = RHI::BindingType::SampledImage, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
                RHI::ShaderBinding{ .type = RHI::BindingType::SampledImage, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
                RHI::ShaderBinding{ .type = RHI::BindingType::SampledImage, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
            };
            auto bindGroupLayout = context.CreateBindGroupLayout(bindGroupLayoutCI);

            m_bindGroup = context.CreateBindGroup(bindGroupLayout);

            RHI::PipelineLayoutCreateInfo pipelineLayoutCI{ bindGroupLayout };
            m_pipelineLayout = context.CreatePipelineLayout(pipelineLayoutCI);

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

            // clang-format off
            RHI::GraphicsPipelineCreateInfo pipelineCI
            {
                .name = "Lighting Pass Pipeline",
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
            // clang-format on
            m_pipeline = context.CreateGraphicsPipeline(pipelineCI);

            context.DestroyBindGroupLayout(bindGroupLayout);

            return ResultCode::Success;
        }

        void Shutdown(RHI::Context& context)
        {
            context.DestroyPipelineLayout(m_pipelineLayout);

            context.DestroyBindGroup(m_bindGroup);
            context.DestroyGraphicsPipeline(m_pipeline);
        }

        ResultCode Setup(RHI::RenderGraph& renderGraph, PassGBuffer& gBuffer, Handle<RHI::ImageAttachment> outputAttachment)
        {
            m_outputAttachment = outputAttachment;

            RHI::PassCreateInfo passCI{};
            passCI.name = "Pass-Lighting";
            passCI.flags = RHI::PassFlags::Graphics;
            m_pass = renderGraph.CreatePass(passCI);

            RHI::ImageViewInfo viewInfo{};
            viewInfo.type = RHI::ImageViewType::View2D;
            viewInfo.subresources.imageAspects = RHI::ImageAspect::Color;
            viewInfo.subresources.arrayCount = 1;
            viewInfo.subresources.mipLevelCount = 1;
            renderGraph.PassUseImage(m_pass, m_outputAttachment, viewInfo, RHI::ImageUsage::Color, RHI::ShaderStage::None, RHI::Access::None);
            renderGraph.PassUseImage(m_pass, gBuffer.m_colorAttachment, viewInfo, RHI::ImageUsage::ShaderResource, RHI::ShaderStage::Pixel, RHI::Access::Read);
            renderGraph.PassUseImage(m_pass, gBuffer.m_normalAttachment, viewInfo, RHI::ImageUsage::ShaderResource, RHI::ShaderStage::Pixel, RHI::Access::Read);
            viewInfo.subresources.imageAspects = RHI::ImageAspect::Depth;
            renderGraph.PassUseImage(m_pass, gBuffer.m_depthAttachment, viewInfo, RHI::ImageUsage::ShaderResource, RHI::ShaderStage::Pixel, RHI::Access::Read);

            return ResultCode::Success;
        }

        void SetupBindings(RHI::Context& context, RHI::RenderGraph& renderGraph, PassGBuffer& gBuffer)
        {
            RHI::BindGroupUpdateInfo bindings[] = {
                RHI::BindGroupUpdateInfo(0, 0, renderGraph.PassGetImageView(m_pass, gBuffer.m_colorAttachment)),
                RHI::BindGroupUpdateInfo(1, 0, renderGraph.PassGetImageView(m_pass, gBuffer.m_normalAttachment)),
                RHI::BindGroupUpdateInfo(2, 0, renderGraph.PassGetImageView(m_pass, gBuffer.m_depthAttachment)),
            };
            context.UpdateBindGroup(m_bindGroup, bindings);
        }

        void Execute(RHI::RenderGraph& renderGraph, RHI::CommandPool& commandPool)
        {
            auto commandList = commandPool.Allocate(RHI::QueueType::Graphics, RHI::CommandListLevel::Primary);

            auto size = renderGraph.GetPassSize(m_pass);
            RHI::Viewport viewport{};
            viewport.width = (float)size.width;
            viewport.height = (float)size.height;
            viewport.maxDepth = 1.0f;
            RHI::Scissor scissor{};
            scissor.width = size.width;
            scissor.height = size.height;

            RHI::CommandListBeginInfo beginInfo{};
            beginInfo.renderGraph = &renderGraph;
            beginInfo.pass = m_pass;
            beginInfo.loadStoreOperations = {
                { .clearValue = { .f32 = { 0.9f, 0.9f, 0.9f, 1.0f } }, .loadOperation = RHI::LoadOperation::Discard, .storeOperation = RHI::StoreOperation::Store }
            };

            // clang-format off
            RHI::DrawInfo drawInfo
            {
                .pipelineState = m_pipeline,
                .bindGroups = { m_bindGroup },
                .vertexBuffers = {},
                .indexBuffer = {},
                .parameters = { 6, 1, 0, 0, 0,}
            };
            // clang-format on

            commandList->Begin(beginInfo);
            commandList->SetViewport(viewport);
            commandList->SetSicssor(scissor);
            commandList->Draw(drawInfo);
            commandList->End();
            renderGraph.Submit(m_pass, commandList);
        }
    };

    class DeferredRenderer final : public Renderer
    {
    public:
        DeferredRenderer() = default;
        ~DeferredRenderer() = default;

        ResultCode OnInit() override
        {
            ResultCode result = ResultCode::Success;

            result = m_passGBuffer.Init(*m_context);
            RHI_ASSERT(IsSucess(result));

            result = m_passLighting.Init(*m_context);
            RHI_ASSERT(IsSucess(result));

            // Setup render graph
            {
                m_renderGraph = m_context->CreateRenderGraph();

                auto outputAttachment = m_renderGraph->ImportSwapchain("Swapchain-Image", *m_swapchain);

                result = m_passGBuffer.Setup(*m_renderGraph);
                RHI_ASSERT(IsSucess(result));

                result = m_passLighting.Setup(*m_renderGraph, m_passGBuffer, outputAttachment);
                RHI_ASSERT(IsSucess(result));

                m_context->CompileRenderGraph(*m_renderGraph);
            }

            RHI::ImageSize2D windowSize = { m_window->GetWindowSize().width, m_window->GetWindowSize().height };
            m_renderGraph->PassResize(m_passGBuffer.m_pass, windowSize);
            m_renderGraph->PassResize(m_passLighting.m_pass, windowSize);

            // Setup bind groups
            {
                m_passLighting.SetupBindings(*m_context, *m_renderGraph, m_passGBuffer);
            }

            return result;
        }

        void OnShutdown() override
        {
            m_passGBuffer.Shutdown(*m_context);
            m_passLighting.Shutdown(*m_context);

            delete m_renderGraph.release();
        }

        void OnRender(const Scene& scene) override
        {
            static uint64_t frameIndex = 0;
            uint32_t i = uint32_t(frameIndex % 2);

            auto commandPool = m_commandPool[i].get();

            auto fence = m_frameFence[i].get();
            if (fence->GetState() != RHI::FenceState::Signaled)
            {
                fence->Wait(UINT64_MAX);
            }
            fence->Reset();

            commandPool->Reset();

            m_passGBuffer.Execute(*m_renderGraph, *commandPool, scene);
            m_passLighting.Execute(*m_renderGraph, *commandPool);
            m_context->ExecuteRenderGraph(*m_renderGraph, fence);

            frameIndex++;
        }

        Ptr<RHI::RenderGraph> m_renderGraph;
        PassGBuffer m_passGBuffer;
        PassLighting m_passLighting;
    };

    Renderer* CreateDeferredRenderer()
    {
        return new DeferredRenderer();
    }

} // namespace Examples