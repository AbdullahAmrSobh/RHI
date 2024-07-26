
#include "Examples-Base/Renderer.hpp"
#include <Examples-Base/FileSystem.hpp>
#include <Examples-Base/Window.hpp>

#include <RHI/Definitions.hpp>
#include <RHI/Format.hpp>
#include <RHI/RHI.hpp>
#include <RHI/Resources.hpp>
#include <cstdint>

namespace Examples
{

    inline static constexpr RHI::ColorValue clearValue = { 1.0f, 0.3f, 0.1f, 1.0f, };

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
        viewInfo.subresources.imageAspects = isDepth ? RHI::ImageAspect::Depth : RHI::ImageAspect::Color;
        viewInfo.subresources.arrayCount = 1;
        viewInfo.subresources.mipLevelCount = 1;
        renderGraph.PassUseImage(pass, imageAttachment, viewInfo, isDepth ? RHI::ImageUsage::Depth : RHI::ImageUsage::Color, RHI::ShaderStage::None, RHI::Access::None);

        return imageAttachment;
    }

    struct PassGBuffer
    {
        Handle<RHI::BindGroup> m_bindGroup;
        Handle<RHI::GraphicsPipeline> m_pipeline;

        Handle<RHI::Pass> m_pass;
        Handle<RHI::ImageAttachment> m_colorAttachment;
        Handle<RHI::ImageAttachment> m_normalAttachment;
        Handle<RHI::ImageAttachment> m_depthAttachment;

        ResultCode Init(RHI::Context& context)
        {
            auto shaderModuleCode = ReadBinaryFile("Shaders/Basic.spv");

            auto shaderMoudule = context.CreateShaderModule(shaderModuleCode);

            RHI::PipelineLayoutCreateInfo pipelineLayoutCI{};
            auto pipelineLayout = context.CreatePipelineLayout(pipelineLayoutCI);

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
                .layout = pipelineLayout,
                .inputAssemblerState = {},
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
            context.DestroyBindGroup(m_bindGroup);
            context.DestroyGraphicsPipeline(m_pipeline);
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

            RHI::ClearValue clearColorValue {};
            clearColorValue.f32 = { 0.4f, 0.3f, 0.1f, 1.0f };

            RHI::ClearValue clearColorValue1 {};
            clearColorValue1.f32 = { 0.3f, 0.4f, 0.2f, 1.0f };

            RHI::ClearValue clearColorValueDepth {};
            clearColorValueDepth.depthStencil.depthValue = 1.0f;

            RHI::CommandListBeginInfo beginInfo{};
            beginInfo.renderGraph = &renderGraph;
            beginInfo.pass = m_pass;
            beginInfo.loadStoreOperations = {
                {  .clearValue = clearColorValue, .loadOperation = RHI::LoadOperation::Discard, .storeOperation = RHI::StoreOperation::Store },
                {  .clearValue = clearColorValue1, .loadOperation = RHI::LoadOperation::Discard, .storeOperation = RHI::StoreOperation::Store },
                {  .clearValue = clearColorValueDepth, .loadOperation = RHI::LoadOperation::Discard, .storeOperation = RHI::StoreOperation::Store },
            };

            // clang-format off
            RHI::DrawInfo drawInfo
            {
                .pipelineState = m_pipeline,
                .bindGroups = { },
                .vertexBuffers = {},
                .indexBuffer = {},
                .parameters ={ 6, 1, 0, 0, 0,}
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

    struct PassLighting
    {
        Handle<RHI::BindGroup> m_bindGroup;
        Handle<RHI::GraphicsPipeline> m_pipeline;

        Handle<RHI::Pass> m_pass;
        Handle<RHI::ImageAttachment> m_outputAttachment;

        ResultCode Init(RHI::Context& context)
        {
            auto shaderModuleCode = ReadBinaryFile("Shaders/Compose.spv");
            auto shaderMoudule = context.CreateShaderModule(shaderModuleCode);

            RHI::BindGroupLayoutCreateInfo bindGroupLayoutCI{
                RHI::ShaderBinding{ .type = RHI::ShaderBindingType::SampledImage, .access = RHI::Access::Read, .arrayCount = 1, .stages = RHI::ShaderStage::Pixel },
            };
            auto bindGroupLayout = context.CreateBindGroupLayout(bindGroupLayoutCI);

            m_bindGroup = context.CreateBindGroup(bindGroupLayout);

            RHI::PipelineLayoutCreateInfo pipelineLayoutCI{ bindGroupLayout };
            auto pipelineLayout = context.CreatePipelineLayout(pipelineLayoutCI);

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
                .layout = pipelineLayout,
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

            return ResultCode::Success;
        }

        void Shutdown(RHI::Context& context)
        {
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
            renderGraph.PassUseImage(m_pass, gBuffer.m_colorAttachment, viewInfo, RHI::ImageUsage::ShaderResource, RHI::ShaderStage::Pixel, RHI::Access::None);
            renderGraph.PassUseImage(m_pass, gBuffer.m_normalAttachment, viewInfo, RHI::ImageUsage::ShaderResource, RHI::ShaderStage::Pixel, RHI::Access::None);
            viewInfo.subresources.imageAspects = RHI::ImageAspect::Depth;
            renderGraph.PassUseImage(m_pass, gBuffer.m_depthAttachment, viewInfo, RHI::ImageUsage::ShaderResource, RHI::ShaderStage::Pixel, RHI::Access::None);

            return ResultCode::Success;
        }

        void SetupBindings(RHI::Context& context, RHI::RenderGraph& renderGraph, PassGBuffer& gBuffer)
        {
            RHI::ResourceBinding bindings[] = {
                RHI::ResourceBinding(0, 0, renderGraph.PassGetImageView(m_pass, gBuffer.m_colorAttachment)),
                // RHI::ResourceBinding(0, 0, renderGraph.PassGetImageView(m_pass, gBuffer.m_normalAttachment)),
                // RHI::ResourceBinding(0, 0, renderGraph.PassGetImageView(m_pass, gBuffer.m_depthAttachment)),
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


            RHI::ClearValue clearColorValue {};
            clearColorValue.f32 = { 0.9f, 0.9f, 0.9f, 1.0f };

            RHI::CommandListBeginInfo beginInfo{};
            beginInfo.renderGraph = &renderGraph;
            beginInfo.pass = m_pass;
            beginInfo.loadStoreOperations = {
                {  .clearValue = clearColorValue, .loadOperation = RHI::LoadOperation::Discard, .storeOperation = RHI::StoreOperation::Store }
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
        ResultCode OnInit() override
        {
            ResultCode result;

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

            return ResultCode::Success;
        }

        void OnShutdown() override
        {
            m_passGBuffer.Shutdown(*m_context);
            m_passLighting.Shutdown(*m_context);
        }

        void OnRender() override
        {
            static uint64_t frameIndex = 0;
            uint32_t i = uint32_t(frameIndex % 2);

            auto commandPool = m_commandPool[i].get();

            auto fence       = m_frameInFlightFence[i].get();
            if (fence->GetState() != RHI::FenceState::Signaled)
            {
                fence->Wait(UINT64_MAX);
            }
            fence->Reset();

            commandPool->Reset();

            m_passGBuffer.Execute(*m_renderGraph, *commandPool);
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