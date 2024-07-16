#include <Examples-Base/ApplicationBase.hpp>
#include <Examples-Base/SceneGraph.hpp>
#include <Examples-Base/Camera.hpp>
#include <Examples-Base/FileSystem.hpp>
#include <Examples-Base/ImGuiRenderer.hpp>

#include <RHI/RHI.hpp>

#include <tracy/Tracy.hpp>

#include <imgui.h>

namespace Examples
{
    class BasicRenderer final : public ApplicationBase
    {
    public:
        Ptr<Camera> m_camera;
        Ptr<Scene> m_scene;

        Shader::SceneTransform m_perFrameData;
        TL::Vector<Shader::ObjectTransform> m_perDrawData;

        Handle<RHI::Sampler> m_sampler;

        Handle<RHI::Buffer> m_perFrameUniformBuffer;
        Handle<RHI::Buffer> m_perObjectUniformBuffer;

        Handle<RHI::BindGroupLayout> m_bindGroupLayout;
        Handle<RHI::BindGroup> m_bindGroup;

        Handle<RHI::PipelineLayout> m_pipelineLayout;
        Handle<RHI::GraphicsPipeline> m_graphicsPipeline;

        Ptr<RHI::RenderGraph> m_renderGraph;
        Handle<RHI::Pass> m_renderPass;

        Ptr<RHI::CommandPool> m_commandPool[2];

        BasicRenderer()
            : ApplicationBase("Hello, Triangle", 1600, 1200)

        {
        }

        void OnInit() override
        {
            ZoneScoped;

            m_camera = RHI::CreatePtr<Camera>();
            m_camera->m_window = m_window.get();
            m_camera->SetPerspective(60.0f, float(m_window->GetWindowSize().width) / float(m_window->GetWindowSize().height), 0.1f, 10000.0f);
            m_camera->SetRotationSpeed(0.0002f);

            m_scene = RHI::CreatePtr<Scene>(m_context.get(), m_launchSettings.sceneFileLocation.c_str());

            m_commandPool[0] = m_context->CreateCommandPool(RHI::CommandPoolFlags::Reset);
            m_commandPool[1] = m_context->CreateCommandPool(RHI::CommandPoolFlags::Reset);

            m_renderGraph = m_context->CreateRenderGraph();

            // setup the render graph
            RHI::PassCreateInfo passCI{
                .name = "GBufferPass",
                .queueType = RHI::QueueType::Graphics,
            };
            m_renderPass = m_renderGraph->CreatePass(passCI);

            auto outputAttachment = m_renderGraph->ImportSwapchain("color-attachment", *m_swapchain);
            auto windowSize = RHI::ImageSize3D{ m_window->GetWindowSize().width, m_window->GetWindowSize().height, 1 };

            RHI::ImageCreateInfo imageCI{
                .name = "depth-attachment",
                .usageFlags = RHI::ImageUsage::DepthStencil,
                .type = RHI::ImageType::Image2D,
                .size = windowSize,
                .format = RHI::Format::D32,
                .sampleCount = RHI::SampleCount::Samples1,
                .mipLevels = 1,
                .arrayCount = 1,
            };
            auto depthAttachment = m_renderGraph->CreateImage(imageCI);

            imageCI.name = "test-attachment";
            imageCI.format = RHI::Format::RGBA32_FLOAT;
            imageCI.usageFlags = RHI::ImageUsage::Color;
            auto imageAttachment = m_renderGraph->CreateImage(imageCI);

            RHI::ImageAttachmentUseInfo attachmentUseInfo {};
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

            SetupGPUResources(m_scene->m_perDrawData);
            LoadPipeline("./Shaders/Basic.spv");
        }

        void OnShutdown() override
        {
            ZoneScoped;

            m_scene->Shutdown(*m_context);
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

            m_camera->SetMovementSpeed(cameraSpeed);
            m_camera->Update(timestep);

            UpdateUniformBuffers(m_camera->GetView(), m_camera->GetProjection());

            auto [windowWidth, windowHeight] = m_window->GetWindowSize();

            // render code
            RHI::Viewport viewport =
            {
                .offsetX = 0,
                .offsetY = 0,
                .width = float(windowWidth),
                .height = float(windowHeight),
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            };

            RHI::Scissor scissor =
            {
                .offsetX = 0,
                .offsetY = 1,
                .width = windowWidth,
                .height = windowHeight
            };

            static uint64_t frameIndex = 0;
            frameIndex++;
            uint32_t i = uint32_t(frameIndex % 2);

            m_commandPool[i]->Reset();
            auto commandList = std::move(m_commandPool[i]->Allocate(RHI::QueueType::Graphics, RHI::CommandListLevel::Primary, 1).front());

            RHI::CommandListBeginInfo beginInfo
            {
                .renderGraph = m_renderGraph.get(),
                .pass = m_renderPass,
                .loadStoreOperations = {},
            };
            commandList->Begin(beginInfo);
            commandList->SetViewport(viewport);
            commandList->SetSicssor(scissor);
            Draw(*commandList);
            m_imguiRenderer->RenderDrawData(ImGui::GetDrawData(), *commandList);

            commandList->End();
            m_renderGraph->SubmitCommands(m_renderPass, commandList);

            m_context->ExecuteRenderGraph(*m_renderGraph);

            (void)m_swapchain->Present();
        }

        void OnEvent(Event& e) override
        {
            m_camera->ProcessEvent(e);
        }

    private:
        void Draw(RHI::CommandList& commandList) const
        {
            ZoneScoped;
            Handle<RHI::BindGroup> bindGroups = { m_bindGroup };

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
    };

} // namespace Examples

int main(int argc, const char* argv[])
{
    using namespace Examples;
    TL::Span args{ argv, (size_t)argc };
    return Entry<BasicRenderer>(args);
}
