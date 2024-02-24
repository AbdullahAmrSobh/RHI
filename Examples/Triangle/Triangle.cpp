#include <Examples-Base/ApplicationBase.hpp>
#include <RHI/RHI.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <tracy/Tracy.hpp>

struct UniformBufferContent
{
    glm::mat4 viewProjection;
};

struct Mesh
{
    void Init(RHI::Context& context, const char* path)
    {
        Assimp::Importer importer{};
        auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals);
        auto mesh = scene->mMeshes[0];

        std::vector<uint32_t> indexBufferData;
        indexBufferData.reserve(mesh->mNumFaces * 3);

        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            for (uint32_t j = 0; j < mesh->mFaces[i].mNumIndices; j++)
            {
                indexBufferData.push_back(mesh->mFaces[i].mIndices[j]);
            }
        }

        std::vector<glm::vec2> texCoordData;
        texCoordData.reserve(mesh->mNumFaces * 3);

        auto uv = mesh->mTextureCoords[0];
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            texCoordData.push_back({ uv[i].x, uv[i].y });
        }

        drawElementsCount = uint32_t(indexBufferData.size());
        indexBuffer = context.CreateBufferWithContentT<uint32_t>(RHI::BufferUsage::Index, indexBufferData).GetValue();
        positionsBuffer = context.CreateBufferWithContentT<aiVector3D>(RHI::BufferUsage::Vertex, { mesh->mVertices, mesh->mNumVertices }).GetValue();
        normalsBuffer = context.CreateBufferWithContentT<aiVector3D>(RHI::BufferUsage::Vertex, { mesh->mNormals, mesh->mNumVertices }).GetValue();
        texCoordBuffer = context.CreateBufferWithContentT<glm::vec2>(RHI::BufferUsage::Vertex, texCoordData).GetValue();
    }

    void Shutdown(RHI::Context& context)
    {
        context.DestroyBuffer(indexBuffer);
        context.DestroyBuffer(positionsBuffer);
        context.DestroyBuffer(normalsBuffer);
        context.DestroyBuffer(texCoordBuffer);
    }

    uint32_t drawElementsCount;

    RHI::Handle<RHI::Buffer> indexBuffer;
    RHI::Handle<RHI::Buffer> positionsBuffer;
    RHI::Handle<RHI::Buffer> normalsBuffer;
    RHI::Handle<RHI::Buffer> texCoordBuffer;
};

class TriangleExample final : public ApplicationBase
{
public:
    TriangleExample()
        : ApplicationBase("Hello, Triangle", 1600, 1200)
    {
    }

    void OnInit() override
    {
        ZoneScopedN("triangle-update");

        m_uniformData.viewProjection = m_camera.GetProjection() * m_camera.GetView();
        m_uniformBuffer = m_context->CreateBufferWithContentT<uint8_t>(RHI::BufferUsage::Uniform, { (uint8_t*)&m_uniformData, sizeof(UniformBufferContent) }).GetValue();
        m_mesh.Init(*m_context, "./Resources/Meshes/simple_cube.obj");

        // create image
        {
            auto textureData = LoadImage("./Resources/Images/image.png");

            RHI::ImageCreateInfo createInfo{};
            createInfo.usageFlags = RHI::ImageUsage::ShaderResource;
            createInfo.usageFlags |= RHI::ImageUsage::CopyDst;
            createInfo.type = RHI::ImageType::Image2D;
            createInfo.size.width = textureData.width;
            createInfo.size.height = textureData.height;
            createInfo.size.depth = 1;
            createInfo.format = RHI::Format::RGBA8_UNORM;
            m_image = m_context->CreateImageWithContent(createInfo, textureData.data).GetValue();
            RHI::ImageViewCreateInfo viewInfo{};
            viewInfo.image = m_image;
            m_imageView = m_context->CreateImageView(viewInfo);
        }

        // create shader bind group layout
        {
            RHI::BindGroupLayoutCreateInfo createInfo = {};
            createInfo.bindings[0] = { RHI::ShaderBindingType::Buffer, RHI::ShaderBindingAccess::OnlyRead, 1, RHI::ShaderStage::Vertex };
            createInfo.bindings[1] = { RHI::ShaderBindingType::Image, RHI::ShaderBindingAccess::OnlyRead, 1, RHI::ShaderStage::Pixel };
            createInfo.bindings[2] = { RHI::ShaderBindingType::Sampler, RHI::ShaderBindingAccess::OnlyRead, 1, RHI::ShaderStage::Pixel };
            m_bindGroupLayout = m_context->CreateBindGroupLayout(createInfo);

            m_pipelineLayout = m_context->CreatePipelineLayout({ m_bindGroupLayout });

            auto shaderCode = ReadBinaryFile("./Resources/Shaders/triangle.spv");
            auto shaderModule = m_context->CreateShaderModule(shaderCode);

            RHI::GraphicsPipelineCreateInfo psoCreateInfo{};
            // clang-format off
            psoCreateInfo.inputAssemblerState.attributes[0] = { .location = 0, .binding = 0, .format = RHI::Format::RGB32_FLOAT, .offset = 0 }; 
            psoCreateInfo.inputAssemblerState.attributes[1] = { .location = 1, .binding = 1, .format = RHI::Format::RGB32_FLOAT, .offset = 0 };  
            psoCreateInfo.inputAssemblerState.attributes[2] = { .location = 2, .binding = 2, .format = RHI::Format::RG32_FLOAT,  .offset = 0 };  
            psoCreateInfo.inputAssemblerState.bindings[0] = { .binding = 0, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex };  
            psoCreateInfo.inputAssemblerState.bindings[1] = { .binding = 1, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex };  
            psoCreateInfo.inputAssemblerState.bindings[2] = { .binding = 2, .stride = RHI::GetFormatByteSize(RHI::Format::RG32_FLOAT),  .stepRate = RHI::PipelineVertexInputRate::PerVertex };
            // clang-format on
            psoCreateInfo.vertexShaderName = "VSMain";
            psoCreateInfo.pixelShaderName = "PSMain";
            psoCreateInfo.vertexShaderModule = shaderModule.get();
            psoCreateInfo.pixelShaderModule = shaderModule.get();
            psoCreateInfo.layout = m_pipelineLayout;
            psoCreateInfo.renderTargetLayout.colorAttachmentsFormats[0] = RHI::Format::BGRA8_UNORM;
            psoCreateInfo.renderTargetLayout.depthAttachmentFormat = RHI::Format::D32;
            psoCreateInfo.depthStencilState.depthTestEnable = true;
            psoCreateInfo.depthStencilState.depthWriteEnable = true;
            psoCreateInfo.depthStencilState.compareOperator = RHI::CompareOperator::Greater;

            m_pipelineState = m_context->CreateGraphicsPipeline(psoCreateInfo);
        }

        // create a sampler state
        {
            m_sampler = m_context->CreateSampler(RHI::SamplerCreateInfo{});

            RHI::BindGroupData bindGroupData{};
            bindGroupData.BindBuffers(0u, m_uniformBuffer);
            bindGroupData.BindImages(1u, m_imageView);
            bindGroupData.BindSamplers(2u, m_sampler);
            m_bindGroup = m_context->CreateBindGroup(m_bindGroupLayout);
            m_context->UpdateBindGroup(m_bindGroup, bindGroupData);
        }

        // setup the render graph
        {
            auto& scheduler = m_context->GetScheduler();
            auto& registry = scheduler.GetRegistry();
            m_renderpass = scheduler.CreatePass("Render-Pass", RHI::QueueType::Graphics);

            auto swapchainAttachment = registry.ImportSwapchainImage("back-buffer", m_swapchain.get());
            m_renderpass->UseColorAttachment(swapchainAttachment, { 0.1f, 0.2f, 0.3f, 1.0f });

            RHI::ImageCreateInfo depthInfo{};
            depthInfo.usageFlags = RHI::ImageUsage::Depth;
            depthInfo.format = RHI::Format::D32;
            depthInfo.type = RHI::ImageType::Image2D;
            auto depthAttachment = registry.CreateTransientImage("depth-buffer", depthInfo);
            m_renderpass->UseDepthAttachment(depthAttachment, { 0.0 });

            scheduler.ResizeFrame({ m_windowWidth, m_windowHeight });
            scheduler.RegisterPass(*m_renderpass);
            scheduler.Compile();
        }
    }

    void OnShutdown() override
    {
        ZoneScopedN("triangle-update");

        m_mesh.Shutdown(*m_context);
        m_context->DestroyImage(m_image);
        m_context->DestroyImageView(m_imageView);

        m_context->DestroyBuffer(m_uniformBuffer);
        m_context->DestroySampler(m_sampler);

        m_context->DestroyPipelineLayout(m_pipelineLayout);
        m_context->DestroyBindGroupLayout(m_bindGroupLayout);
        m_context->DestroyGraphicsPipeline(m_pipelineState);
        m_context->DestroyBindGroup(m_bindGroup);
    }

    void OnUpdate(Timestep timestep) override
    {
        ZoneScopedN("triangle-update");

        m_camera.Update(timestep);

        auto projection = m_camera.GetProjection() * m_camera.GetView();
        auto ptr = m_context->MapBuffer(m_uniformBuffer);
        memcpy(ptr, &projection, sizeof(glm::mat4));
        m_context->UnmapBuffer(m_uniformBuffer);

        (void)timestep;

        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();

        Render();
        m_swapchain->Present();
    }

    void Render()
    {
        auto& scheduler = m_context->GetScheduler();
        scheduler.Begin();

        auto commandList = m_commandListAllocator->Allocate();

        RHI::Viewport viewport = {
            .width = float(m_windowWidth),
            .height = float(m_windowHeight),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        RHI::Scissor scissor = {
            .width = m_windowWidth,
            .height = m_windowHeight
        };

        RHI::DrawInfo drawCommand = {
            .pipelineState = m_pipelineState,
            .bindGroups = m_bindGroup,
            .vertexBuffers = { m_mesh.positionsBuffer, m_mesh.normalsBuffer, m_mesh.texCoordBuffer },
            .indexBuffers = m_mesh.indexBuffer,
            .parameters = { .elementCount = m_mesh.drawElementsCount },
        };

        commandList->Begin(*m_renderpass);
        commandList->SetViewport(viewport);
        commandList->SetSicssor(scissor);
        commandList->Draw(drawCommand);

        // auto drawData = ImGui::GetDrawData();
        // m_imguiRenderer->RenderDrawData(drawData, *commandList);
        commandList->End();

        scheduler.End();
    }

private:
    UniformBufferContent m_uniformData;

    RHI::Handle<RHI::BindGroupLayout> m_bindGroupLayout;

    RHI::Handle<RHI::BindGroup> m_bindGroup;

    RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;

    RHI::Handle<RHI::GraphicsPipeline> m_pipelineState;

    RHI::Handle<RHI::Buffer> m_uniformBuffer;

    RHI::Handle<RHI::Image> m_image;
    RHI::Handle<RHI::ImageView> m_imageView;

    RHI::Handle<RHI::Sampler> m_sampler;

    Mesh m_mesh;

    std::unique_ptr<RHI::Pass> m_renderpass;
};

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    return ApplicationBase::Entry<TriangleExample>();
}