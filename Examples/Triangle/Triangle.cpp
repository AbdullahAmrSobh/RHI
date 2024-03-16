#include <Examples-Base/ApplicationBase.hpp>
#include <RHI/RHI.hpp>
#include <RHI/Pass.hpp>

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
        indexBuffer = context.CreateBuffer<uint32_t>(RHI::BufferUsage::Index, indexBufferData).GetValue();
        positionsBuffer = context.CreateBuffer<aiVector3D>(RHI::BufferUsage::Vertex, { mesh->mVertices, mesh->mNumVertices }).GetValue();
        normalsBuffer = context.CreateBuffer<aiVector3D>(RHI::BufferUsage::Vertex, { mesh->mNormals, mesh->mNumVertices }).GetValue();
        texCoordBuffer = context.CreateBuffer<glm::vec2>(RHI::BufferUsage::Vertex, texCoordData).GetValue();
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
        ZoneScoped;

        m_uniformData.viewProjection = m_camera.GetProjection() * m_camera.GetView();
        m_uniformBuffer = m_context->CreateBuffer(RHI::BufferUsage::Uniform, RHI::TL::ToBytes(m_uniformData)).GetValue();
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
            m_image = m_context->CreateImage(createInfo, textureData.data).GetValue();
            RHI::ImageViewCreateInfo viewInfo{};
            viewInfo.image = m_image;
            m_imageView = m_context->CreateImageView(viewInfo);
        }

        // create shader bind group layout
        {
            RHI::BindGroupLayoutCreateInfo createInfo = {};
            createInfo.bindings[0] = { RHI::ShaderBindingType::UniformBuffer, RHI::Access::Read, 1, RHI::ShaderStage::Vertex };
            createInfo.bindings[1] = { RHI::ShaderBindingType::SampledImage, RHI::Access::Read, 1, RHI::ShaderStage::Pixel };
            createInfo.bindings[2] = { RHI::ShaderBindingType::Sampler, RHI::Access::Read, 1, RHI::ShaderStage::Pixel };
            m_renderBindGroupLayout = m_context->CreateBindGroupLayout(createInfo);
            m_renderPipelineLayout = m_context->CreatePipelineLayout({ m_renderBindGroupLayout });
        }
        {
            auto shaderCode = ReadBinaryFile("./Resources/Shaders/triangle.spv");
            auto shaderModule = m_context->CreateShaderModule(shaderCode);
            RHI::GraphicsPipelineCreateInfo createInfo{};
            // clang-format off
            createInfo.inputAssemblerState.attributes[0] = { .location = 0, .binding = 0, .format = RHI::Format::RGB32_FLOAT, .offset = 0 }; 
            createInfo.inputAssemblerState.attributes[1] = { .location = 1, .binding = 1, .format = RHI::Format::RGB32_FLOAT, .offset = 0 };  
            createInfo.inputAssemblerState.attributes[2] = { .location = 2, .binding = 2, .format = RHI::Format::RG32_FLOAT,  .offset = 0 };  
            createInfo.inputAssemblerState.bindings[0] = { .binding = 0, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex };  
            createInfo.inputAssemblerState.bindings[1] = { .binding = 1, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex };  
            createInfo.inputAssemblerState.bindings[2] = { .binding = 2, .stride = RHI::GetFormatByteSize(RHI::Format::RG32_FLOAT),  .stepRate = RHI::PipelineVertexInputRate::PerVertex };
            // clang-format on
            createInfo.vertexShaderName = "VSMain";
            createInfo.pixelShaderName = "PSMain";
            createInfo.vertexShaderModule = shaderModule.get();
            createInfo.pixelShaderModule = shaderModule.get();
            createInfo.layout = m_renderPipelineLayout;
            createInfo.renderTargetLayout.colorAttachmentsFormats[0] = RHI::Format::BGRA8_UNORM;
            createInfo.renderTargetLayout.depthAttachmentFormat = RHI::Format::D32;
            createInfo.depthStencilState.depthTestEnable = true;
            createInfo.depthStencilState.depthWriteEnable = true;
            createInfo.depthStencilState.compareOperator = RHI::CompareOperator::Greater;
            m_renderPipelineState = m_context->CreateGraphicsPipeline(createInfo);
        }
        // create shader bind group layout compute pipeline
        {
            RHI::BindGroupLayoutCreateInfo createInfo = {};
            createInfo.bindings[0] = { RHI::ShaderBindingType::StorageImage, RHI::Access::ReadWrite, 1, RHI::ShaderStage::Compute };
            m_computeBindGroupLayout = m_context->CreateBindGroupLayout(createInfo);
            m_computePipelineLayout = m_context->CreatePipelineLayout({ m_computeBindGroupLayout });
        }
        {
            auto shaderCode = ReadBinaryFile("./Resources/Shaders/compute.spv");
            auto shaderModule = m_context->CreateShaderModule(shaderCode);
            RHI::ComputePipelineCreateInfo createInfo{};
            createInfo.shaderName = "CSMain";
            createInfo.shaderModule = shaderModule.get();
            createInfo.layout = m_computePipelineLayout;
            m_computePipelineState = m_context->CreateComputePipeline(createInfo);
        }
        // create shader bind group compose pipeline
        {
            RHI::BindGroupLayoutCreateInfo createInfo = {};
            createInfo.bindings[0] = { RHI::ShaderBindingType::SampledImage, RHI::Access::Read, 1, RHI::ShaderStage::Pixel };
            createInfo.bindings[1] = { RHI::ShaderBindingType::SampledImage, RHI::Access::ReadWrite, 1, RHI::ShaderStage::Pixel };
            createInfo.bindings[2] = { RHI::ShaderBindingType::Sampler, RHI::Access::Read, 1, RHI::ShaderStage::Pixel };
            createInfo.bindings[3] = { RHI::ShaderBindingType::UniformBuffer, RHI::Access::Read, 1, RHI::ShaderStage::Pixel };
            m_composeBindGroupLayout = m_context->CreateBindGroupLayout(createInfo);
            m_composePipelineLayout = m_context->CreatePipelineLayout({ m_composeBindGroupLayout });
        }
        {
            auto shaderCode = ReadBinaryFile("./Resources/Shaders/compose.spv");
            auto shaderModule = m_context->CreateShaderModule(shaderCode);
            RHI::GraphicsPipelineCreateInfo createInfo{};
            createInfo.vertexShaderName = "VSMain";
            createInfo.pixelShaderName = "PSMain";
            createInfo.vertexShaderModule = shaderModule.get();
            createInfo.pixelShaderModule = shaderModule.get();
            createInfo.layout = m_composePipelineLayout;
            createInfo.renderTargetLayout.colorAttachmentsFormats[0] = RHI::Format::BGRA8_UNORM;
            createInfo.renderTargetLayout.depthAttachmentFormat = RHI::Format::D32;
            createInfo.depthStencilState.depthTestEnable = true;
            createInfo.depthStencilState.depthWriteEnable = true;
            createInfo.depthStencilState.compareOperator = RHI::CompareOperator::Greater;
            m_composePipelineState = m_context->CreateGraphicsPipeline(createInfo);
        }

        // setup the render graph
        // RHI::ImagePassAttachment* colorAttachment;
        // RHI::ImagePassAttachment* maskAttachment;
        {
            auto& scheduler = m_context->GetScheduler();

            m_renderPass = scheduler.CreatePass("Render-Pass", RHI::QueueType::Graphics);
            m_renderPass->SetRenderTargetSize({ 1600, 1200 });
            m_renderPass->CreateRenderTarget("depth-target", RHI::Format::D32, RHI::DepthStencilValue{ 0.0f });
            // colorAttachment = m_renderPass->UseRenderTarget("color-target", m_swapchain.get(), RHI::ColorValue{ 0.0f, 0.2f, 0.3f, 1.0f });
            m_renderPass->UseRenderTarget("color-target", m_swapchain.get(), RHI::ColorValue{ 0.0f, 0.2f, 0.3f, 1.0f });

            // m_computePass = scheduler.CreatePass("Compute-Pass", RHI::QueueType::Compute);
            // maskAttachment = m_computePass->CreateTransientImage("mask", RHI::Format::R8_UNORM, RHI::ImageUsage::StorageResource, RHI::ImageSize2D{ m_windowWidth, m_windowHeight });

            // m_composePass = scheduler.CreatePass("Compose-Pass", RHI::QueueType::Graphics);
            // m_composePass->SetRenderTargetSize({ 1600, 1200 });
            // m_composePass->UseRenderTarget(colorAttachment, RHI::ColorValue{ 0.0f });
            // m_composePass->UseImageResource(maskAttachment, RHI::ImageUsage::ShaderResource, RHI::Access::Read);
            scheduler.Compile();
        }

        m_sampler = m_context->CreateSampler(RHI::SamplerCreateInfo{});

        RHI::BindGroupData bindGroupData{};
        bindGroupData.BindBuffers(0u, m_uniformBuffer);
        bindGroupData.BindImages(1u, m_imageView);
        bindGroupData.BindSamplers(2u, m_sampler);
        m_renderBindGroup = m_context->CreateBindGroup(m_renderBindGroupLayout);
        m_context->UpdateBindGroup(m_renderBindGroup, bindGroupData);

        // bindGroupData = RHI::BindGroupData{};
        // maskAttachment->m_stage |= RHI::ShaderStage::Compute;
        // bindGroupData.BindImages(0u, maskAttachment->m_view);
        // m_computeBindGroup = m_context->CreateBindGroup(m_computeBindGroupLayout);
        // m_context->UpdateBindGroup(m_computeBindGroup, bindGroupData);

        // bindGroupData = RHI::BindGroupData {};
        // bindGroupData.BindImages(0u, maskAttachment->m_view);
        // bindGroupData.BindImages(1u, colorAttachment->m_view);
        // bindGroupData.BindSamplers(2u, m_sampler);
        // bindGroupData.BindBuffers(3u, m_uniformBuffer);
        // m_composeBindGroup = m_context->CreateBindGroup(m_composeBindGroupLayout);
        // m_context->UpdateBindGroup(m_composeBindGroup, bindGroupData);
    }

    void OnShutdown() override
    {
        ZoneScoped;

        m_mesh.Shutdown(*m_context);
        m_context->DestroyImage(m_image);
        m_context->DestroyImageView(m_imageView);

        m_context->DestroyBuffer(m_uniformBuffer);
        m_context->DestroySampler(m_sampler);

        m_context->DestroyBindGroupLayout(m_renderBindGroupLayout);
        m_context->DestroyPipelineLayout(m_renderPipelineLayout);
        m_context->DestroyGraphicsPipeline(m_renderPipelineState);
        m_context->DestroyBindGroup(m_renderBindGroup);

        m_context->DestroyBindGroupLayout(m_computeBindGroupLayout);
        m_context->DestroyPipelineLayout(m_computePipelineLayout);
        m_context->DestroyComputePipeline(m_computePipelineState);
        m_context->DestroyBindGroup(m_computeBindGroup);

        m_context->DestroyBindGroupLayout(m_composeBindGroupLayout);
        m_context->DestroyPipelineLayout(m_composePipelineLayout);
        m_context->DestroyGraphicsPipeline(m_composePipelineState);
        m_context->DestroyBindGroup(m_composeBindGroup);
    }

    void OnUpdate(Timestep timestep) override
    {
        ZoneScoped;

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

        RHI::Viewport viewport = {};
        viewport.width = float(m_windowWidth);
        viewport.height = float(m_windowHeight);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        RHI::Scissor scissor = {};
        scissor.width = m_windowWidth;
        scissor.height = m_windowHeight;
        {
            RHI::DrawInfo drawCommand = {};
            drawCommand.pipelineState = m_renderPipelineState;
            drawCommand.bindGroups = m_renderBindGroup;
            drawCommand.vertexBuffers = { m_mesh.positionsBuffer, m_mesh.normalsBuffer, m_mesh.texCoordBuffer };
            drawCommand.indexBuffers = m_mesh.indexBuffer;
            drawCommand.parameters = { .elementCount = m_mesh.drawElementsCount };

            auto commandList = m_commandListAllocator->Allocate(RHI::QueueType::Graphics);
            commandList->Begin(*m_renderPass);
            commandList->SetViewport(viewport);
            commandList->SetSicssor(scissor);
            commandList->Draw(drawCommand);
            commandList->End();
            m_renderPass->SubmitCommandList(commandList);
            m_commandListAllocator->Release(commandList);
        }

        // {
        //     RHI::DispatchInfo dispatchInfo{};
        //     dispatchInfo.pipelineState = m_computePipelineState;
        //     dispatchInfo.bindGroups = m_computeBindGroup;
        //     dispatchInfo.parameters.countX = 32;
        //     dispatchInfo.parameters.countY = 32;
        //     dispatchInfo.parameters.countZ = 32;
        //     auto commandList = m_commandListAllocator->Allocate();
        //     commandList->Begin(*m_computePass);
        //     commandList->Dispatch(dispatchInfo);
        //     commandList->End();
        //     m_computePass->SubmitCommandList(commandList);
        // }

        // {
        //     RHI::DrawInfo drawCommand = {};
        //     drawCommand.pipelineState = m_renderPipelineState;
        //     drawCommand.bindGroups = m_renderBindGroup;
        //     drawCommand.parameters = { .elementCount = 6 };

        //     auto commandList = m_commandListAllocator->Allocate();
        //     commandList->Begin(*m_renderPass);
        //     commandList->SetViewport(viewport);
        //     commandList->SetSicssor(scissor);
        //     commandList->Draw(drawCommand);
        //     commandList->End();
        //     m_composePass->SubmitCommandList(commandList);
        // }

        scheduler.End();
    }

private:
    UniformBufferContent m_uniformData;

    RHI::Handle<RHI::Buffer> m_uniformBuffer;
    RHI::Handle<RHI::Image> m_image;
    RHI::Handle<RHI::ImageView> m_imageView;
    RHI::Handle<RHI::Sampler> m_sampler;

    Mesh m_mesh;

    RHI::Handle<RHI::BindGroupLayout> m_renderBindGroupLayout;
    RHI::Handle<RHI::PipelineLayout> m_renderPipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline> m_renderPipelineState;
    RHI::Handle<RHI::BindGroup> m_renderBindGroup;

    RHI::Handle<RHI::BindGroupLayout> m_computeBindGroupLayout;
    RHI::Handle<RHI::PipelineLayout> m_computePipelineLayout;
    RHI::Handle<RHI::ComputePipeline> m_computePipelineState;
    RHI::Handle<RHI::BindGroup> m_computeBindGroup;

    RHI::Handle<RHI::BindGroupLayout> m_composeBindGroupLayout;
    RHI::Handle<RHI::PipelineLayout> m_composePipelineLayout;
    RHI::Handle<RHI::GraphicsPipeline> m_composePipelineState;
    RHI::Handle<RHI::BindGroup> m_composeBindGroup;

    RHI::Ptr<RHI::Pass> m_renderPass;
    RHI::Ptr<RHI::Pass> m_computePass;
    RHI::Ptr<RHI::Pass> m_composePass;
};

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    return ApplicationBase::Entry<TriangleExample>();
}
