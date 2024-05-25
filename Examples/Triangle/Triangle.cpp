#include <Examples-Base/ApplicationBase.hpp>
#include <Examples-Base/SceneGraph.hpp>

#include <RHI/RHI.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include <tracy/Tracy.hpp>

inline static constexpr const char* DEFAULT_SCENE_PATH = "C:/Users/abdul/Desktop/Main.1_Sponza/NewSponza_Main_glTF_002.gltf";

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

        RHI::ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.name = "depth-attachment";
        imageCreateInfo.format = RHI::Format::D32;
        imageCreateInfo.usageFlags = RHI::ImageUsage::DepthStencil;
        imageCreateInfo.type = RHI::ImageType::Image2D;
        imageCreateInfo.size.width = m_windowWidth;
        imageCreateInfo.size.height = m_windowHeight;
        imageCreateInfo.size.depth = 1;
        auto depthAttachment = m_renderGraph->CreateImage(imageCreateInfo);
        imageCreateInfo.name = "test-attachment";
        imageCreateInfo.format = RHI::Format::RGBA32_FLOAT;
        imageCreateInfo.usageFlags = RHI::ImageUsage::Color;
        auto imageAttachment = m_renderGraph->CreateImage(imageCreateInfo);

        RHI::ImageAttachmentUseInfo attachmentUseInfo{};
        attachmentUseInfo.usage = RHI::ImageUsage::Color;
        attachmentUseInfo.loadStoreOperations.loadOperation = RHI::LoadOperation::Discard;
        attachmentUseInfo.loadStoreOperations.storeOperation = RHI::StoreOperation::Store;
        attachmentUseInfo.clearValue = { 0.0f, 0.2f, 0.3f, 1.0f };
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

        m_commandList[0] = m_commandPool->Allocate(RHI::QueueType::Graphics, RHI::CommandListLevel::Primary);
        m_commandList[1] = m_commandPool->Allocate(RHI::QueueType::Graphics, RHI::CommandListLevel::Primary);

        SetupGPUResources(m_scene->m_perDrawData);
        LoadPipeline("./Shaders/Basic.spv");
    }

    void OnShutdown() override
    {
        ZoneScoped;

        m_scene->Shutdown(*m_context);

        m_commandPool->Release(m_commandList[0]);
        m_commandPool->Release(m_commandList[1]);
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

        m_commandPool->Reset();

        m_commandList[i]->Begin(*m_renderGraph, m_renderPass, {}, {});
        m_commandList[i]->SetViewport(viewport);
        m_commandList[i]->SetSicssor(scissor);
        Draw(*m_commandList[i]);
        m_imguiRenderer->RenderDrawData(ImGui::GetDrawData(), *m_commandList[i]);

        m_commandList[i]->End();
        m_renderGraph->SubmitCommands(m_renderPass, m_commandList[i]);

        m_context->ExecuteRenderGraph(*m_renderGraph);

        m_swapchain->Present();
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
                drawInfo.bindGroups = {{m_bindGroup, uint32_t(sizeof(Shader::PerDraw) * nodeIndex)}};
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
        memcpy(ptr, &m_perFrameData, sizeof(Shader::PerFrame));
        m_context->UnmapBuffer(m_perFrameUniformBuffer);
    }

    void SetupGPUResources(TL::Span<Shader::PerDraw> perDraw)
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
        bufferCreateInfo.byteSize = sizeof(Shader::PerFrame);
        m_perFrameUniformBuffer = m_context->CreateBuffer(bufferCreateInfo).GetValue();
        bufferCreateInfo.byteSize = perDraw.size_bytes();
        m_perObjectUniformBuffer = m_context->CreateBuffer(bufferCreateInfo).GetValue();

        auto ptr = m_context->MapBuffer(m_perObjectUniformBuffer);
        memcpy(ptr, perDraw.data(), perDraw.size() * sizeof(Shader::PerDraw));
        m_context->UnmapBuffer(m_perObjectUniformBuffer);

        // update bind groups
        TL::Span<const RHI::ResourceBinding> bindings{
            RHI::ResourceBinding(0, 0, m_perFrameUniformBuffer),
            RHI::ResourceBinding(1, 0, RHI::ResourceBinding::DynamicBufferBinding(m_perObjectUniformBuffer, 0, sizeof(Shader::PerDraw))),
        };
        m_context->UpdateBindGroup(m_bindGroup, bindings);
    }

    void LoadPipeline(const char* shaderPath)
    {
        auto shaderCode = ReadBinaryFile(shaderPath);
        auto shaderModule = m_context->CreateShaderModule(shaderCode);
        RHI::GraphicsPipelineCreateInfo createInfo{};
        createInfo.inputAssemblerState.attributes[0] = { .location = 0, .binding = 0, .format = RHI::Format::RGB32_FLOAT, .offset = 0 };
        createInfo.inputAssemblerState.attributes[1] = { .location = 1, .binding = 1, .format = RHI::Format::RGB32_FLOAT, .offset = 0 };
        // createInfo.inputAssemblerState.attributes[2] = { .location = 2, .binding = 2, .format = RHI::Format::RG32_FLOAT,  .offset = 0 };
        createInfo.inputAssemblerState.bindings[0] = { .binding = 0, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex };
        createInfo.inputAssemblerState.bindings[1] = { .binding = 1, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex };
        // createInfo.inputAssemblerState.bindings[2] = { .binding = 2, .stride = RHI::GetFormatByteSize(RHI::Format::RG32_FLOAT),  .stepRate = RHI::PipelineVertexInputRate::PerVertex };
        createInfo.vertexShaderName = "VSMain";
        createInfo.pixelShaderName = "PSMain";
        createInfo.vertexShaderModule = shaderModule.get();
        createInfo.pixelShaderModule = shaderModule.get();
        createInfo.layout = m_pipelineLayout;
        createInfo.renderTargetLayout.colorAttachmentsFormats[0] = RHI::Format::BGRA8_UNORM;
        createInfo.renderTargetLayout.colorAttachmentsFormats[1] = RHI::Format::RGBA32_FLOAT;
        createInfo.renderTargetLayout.depthAttachmentFormat = RHI::Format::D32;
        createInfo.depthStencilState.depthTestEnable = true;
        createInfo.depthStencilState.depthWriteEnable = true;
        createInfo.depthStencilState.compareOperator = RHI::CompareOperator::Less;
        m_graphicsPipeline = m_context->CreateGraphicsPipeline(createInfo);
    }

private:
    Shader::PerFrame m_perFrameData = {};
    TL::Vector<Shader::PerDraw> m_perDrawData; // todo: make as array

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

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    return ApplicationBase::Entry<BasicRenderer>();
}
