#include <Examples-Base/ApplicationBase.hpp>
#include <RHI/RHI.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct UniformBufferContent
{
    glm::mat4 viewProjection;
};

struct Mesh
{
    uint32_t drawElementsCount;

    RHI::Handle<RHI::Buffer> indexBuffer;
    RHI::Handle<RHI::Buffer> positionsBuffer;
    RHI::Handle<RHI::Buffer> normalsBuffer;
    RHI::Handle<RHI::Buffer> texCoordBuffer;

    RHI::ImageSize3D textureSize;
    RHI::Handle<RHI::Buffer> textureStagingBuffer;
};

class TriangleExample final : public ApplicationBase
{
public:
    TriangleExample()
        : ApplicationBase("Hello, Triangle", 1600, 1200)
    {
    }

    template<typename T>
    RHI::Handle<RHI::Buffer> CreateBuffer(RHI::TL::Span<T> data, RHI::Flags<RHI::BufferUsage> usageFlags)
    {
        auto createInfo = RHI::BufferCreateInfo{};
        createInfo.usageFlags = usageFlags;
        createInfo.byteSize = data.size() * sizeof(T);

        auto buffer = m_bufferPool->Allocate(createInfo).GetValue();
        RHI::DeviceMemoryPtr bufferPtr = m_bufferPool->MapBuffer(buffer);
        RHI_ASSERT(bufferPtr != nullptr);
        memcpy(bufferPtr, data.data(), data.size() * sizeof(T));
        m_bufferPool->UnmapBuffer(buffer);

        return buffer;
    }

    Mesh LoadScene(const char* path)
    {
        Assimp::Importer importer{};
        auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals);
        auto mesh = scene->mMeshes[0];
        auto texture = LoadImage("./Resources/Images/image.png");

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

        Mesh result{
            .drawElementsCount = uint32_t(indexBufferData.size()),
            .indexBuffer = CreateBuffer<uint32_t>(indexBufferData, RHI::BufferUsage::Index),
            .positionsBuffer = CreateBuffer<aiVector3D>({ mesh->mVertices, mesh->mNumVertices }, RHI::BufferUsage::Vertex),
            .normalsBuffer = CreateBuffer<aiVector3D>({ mesh->mNormals, mesh->mNumVertices }, RHI::BufferUsage::Vertex),
            .texCoordBuffer = CreateBuffer<glm::vec2>(texCoordData, RHI::BufferUsage::Vertex),
            .textureSize = {
                .width = texture.width,
                .height = texture.height,
            },
            .textureStagingBuffer = CreateBuffer<uint8_t>({ texture.data.data(), texture.data.size() }, RHI::BufferUsage::CopySrc)
        };
        return result;
    }

    void SetupPipelines(RHI::Handle<RHI::BindGroupLayout> bindGroupLayout)
    {
        m_pipelineLayout = m_context->CreatePipelineLayout({ bindGroupLayout });

        auto shaderCode = ReadBinaryFile("./Resources/Shaders/triangle.spv");

        RHI::ShaderModuleCreateInfo createInfo{};
        createInfo.code = shaderCode.data();
        createInfo.size = shaderCode.size();

        auto shaderModule = m_context->CreateShaderModule(createInfo);

        RHI::GraphicsPipelineCreateInfo psoCreateInfo{};
        // clang-format off
        psoCreateInfo.inputAssemblerState.attributes = {  
            { .location = 0, .binding = 0, .format = RHI::Format::RGB32_FLOAT, .offset = 0,  }, 
            { .location = 1, .binding = 1, .format = RHI::Format::RGB32_FLOAT, .offset = 0,  },  
            { .location = 2, .binding = 2, .format = RHI::Format::RG32_FLOAT, .offset = 0,  },  
        };  
        psoCreateInfo.inputAssemblerState.bindings = { 
            { .binding = 0, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex,  },  
            { .binding = 1, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex,  },  
            { .binding = 2, .stride = RHI::GetFormatByteSize(RHI::Format::RG32_FLOAT),  .stepRate = RHI::PipelineVertexInputRate::PerVertex,  },
        };
        // clang-format on
        psoCreateInfo.vertexShaderModule = shaderModule.get();
        psoCreateInfo.vertexShaderName = "VSMain";
        psoCreateInfo.pixelShaderModule = shaderModule.get();
        psoCreateInfo.pixelShaderName = "PSMain";
        psoCreateInfo.layout = m_pipelineLayout;
        psoCreateInfo.renderTargetLayout.colorAttachmentsFormats = { RHI::Format::BGRA8_UNORM };
        psoCreateInfo.renderTargetLayout.depthAttachmentFormat = RHI::Format::D32;
        psoCreateInfo.depthStencilState.depthTestEnable = true;
        psoCreateInfo.depthStencilState.depthWriteEnable = true;
        psoCreateInfo.depthStencilState.compareOperator = RHI::CompareOperator::Greater;

        m_pipelineState = m_context->CreateGraphicsPipeline(psoCreateInfo);
    }

    void SetupRenderGraph()
    {
        auto& frameGraph = *m_frameScheduler.get();
        auto& registry = frameGraph.GetRegistry();
        m_renderpass = m_context->CreatePass("Render-Pass", RHI::QueueType::Graphics);

        RHI::ImageCreateInfo depthInfo{};
        depthInfo.usageFlags = RHI::ImageUsage::Depth;
        depthInfo.format = RHI::Format::D32;
        depthInfo.type = RHI::ImageType::Image2D;
        m_swapchainAttachment = registry.ImportSwapchainImage("back-buffer", m_swapchain.get());
        auto depthAttachment = registry.CreateTransientImage("depth-buffer", depthInfo);
        m_renderpass->UseColorAttachment(m_swapchainAttachment, { 0.1f, 0.2f, 0.3f, 1.0f });
        m_renderpass->UseDepthAttachment(depthAttachment, { 0.0 });

        // auto imguiPass = SetupImguiPass(m_swapchainAttachment, depthAttachment);

        frameGraph.RegisterPass(*m_renderpass);
        // frameGraph.RegisterPass(*imguiPass);

        frameGraph.ResizeFrame({ m_windowWidth, m_windowHeight });
        frameGraph.Compile();
    }

    void OnInit() override
    {
        {
            m_uniformData.viewProjection = m_camera.GetProjection() * m_camera.GetView();
        }

        // create buffer resource pool
        {
            // create buffer resource
            m_mesh = LoadScene("./Resources/Meshes/simple_cube.obj");
            m_uniformBuffer = CreateBuffer<UniformBufferContent>(m_uniformData, RHI::BufferUsage::Uniform);
        }

        // create image
        {
            RHI::ImageCreateInfo createInfo{};
            createInfo.usageFlags = RHI::ImageUsage::ShaderResource | RHI::ImageUsage::CopyDst;
            createInfo.type = RHI::ImageType::Image2D;
            createInfo.size = m_mesh.textureSize;
            createInfo.size.depth = 1;
            createInfo.format = RHI::Format::RGBA8_UNORM;
            createInfo.sampleCount = RHI::SampleCount::Samples1;
            createInfo.mipLevels = 1;
            createInfo.arrayCount = 1;
            m_image = m_imagePool->Allocate(createInfo).GetValue();
            RHI::ImageViewCreateInfo viewInfo{};
            m_imageView = m_context->CreateImageView(m_image, viewInfo);
        }

        // upload image data to the gpu
        {
            auto fence = m_context->CreateFence();

            // copy data from the staging buffer to the image.
            RHI::CopyBufferToImageDescriptor copyCommand = {};
            copyCommand.srcBuffer = m_mesh.textureStagingBuffer;
            copyCommand.srcOffset = 0;
            // copyCommand.srcBytesPerRow;
            // copyCommand.srcBytesPerImage;
            copyCommand.srcSize = m_mesh.textureSize;
            copyCommand.srcSize.depth = 1;
            copyCommand.dstImage = m_image;
            copyCommand.dstOffset.x = 0;
            copyCommand.dstOffset.y = 0;
            copyCommand.dstOffset.z = 0;
            auto commandList = m_commandListAllocator->Allocate();
            commandList->Begin();
            commandList->Submit(copyCommand);
            commandList->End();
            m_frameScheduler->ExecuteCommandList(commandList, *fence);
            fence->Wait(UINT64_MAX);
            m_bufferPool->FreeBuffer(m_mesh.textureStagingBuffer);
        }

        // create shader bind group layout
        {
            RHI::BindGroupLayoutCreateInfo createInfo = {
                { RHI::ShaderBindingType::Buffer, RHI::ShaderBindingAccess::OnlyRead, 1, RHI::ShaderStage::Vertex },
                { RHI::ShaderBindingType::Image, RHI::ShaderBindingAccess::OnlyRead, 1, RHI::ShaderStage::Pixel },
                { RHI::ShaderBindingType::Sampler, RHI::ShaderBindingAccess::OnlyRead, 1, RHI::ShaderStage::Pixel },
            };
            m_bindGroupLayout = m_context->CreateBindGroupLayout(createInfo);
        }

        SetupPipelines(m_bindGroupLayout);

        SetupRenderGraph();

        // create a sampler state
        {
            RHI::SamplerCreateInfo samplerCreateInfo{};
            m_sampler = m_context->CreateSampler(samplerCreateInfo);

            RHI::BindGroupData bindGroupData{};
            bindGroupData.BindBuffers(0u, m_uniformBuffer);
            bindGroupData.BindImages(1u, m_imageView);
            bindGroupData.BindSamplers(2u, m_sampler);
            m_bindGroup = m_bindGroupAllocator->AllocateBindGroups(m_bindGroupLayout).front();
            m_bindGroupAllocator->Update(m_bindGroup, bindGroupData);
        }
    }

    void OnShutdown() override
    {
        m_bindGroupAllocator->Free(m_bindGroup);

        m_context->DestroyBindGroupLayout(m_bindGroupLayout);
        m_context->DestroyPipelineLayout(m_pipelineLayout);
        m_context->DestroyGraphicsPipeline(m_pipelineState);

        m_bufferPool->FreeBuffer(m_mesh.indexBuffer);
        m_bufferPool->FreeBuffer(m_mesh.positionsBuffer);
        m_bufferPool->FreeBuffer(m_uniformBuffer);

        m_imagePool->FreeImage(m_image);
    }

    void OnUpdate(Timestep timestep) override
    {
        m_camera.Update(timestep);

        auto projection = m_camera.GetProjection() * m_camera.GetView();
        auto ptr = m_bufferPool->MapBuffer(m_uniformBuffer);
        memcpy(ptr, &projection, sizeof(glm::mat4));
        m_bufferPool->UnmapBuffer(m_uniformBuffer);

        (void)timestep;

        ImGui::NewFrame();

        ImGui::ShowDemoWindow();


        ImGui::Render();

        Render();
        m_swapchain->Present();
    }

    void Render()
    {
        m_frameScheduler->Begin();

        auto currentFrameIndex = m_frameScheduler->GetCurrentFrameIndex();

        m_commandListAllocator->Flush(currentFrameIndex);
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

        RHI::CommandDraw drawCommand = {
            .pipelineState = m_pipelineState,
            .bindGroups = m_bindGroup,
            .vertexBuffers = { m_mesh.positionsBuffer, m_mesh.normalsBuffer, m_mesh.texCoordBuffer },
            .indexBuffers = m_mesh.indexBuffer,
            .parameters = { .elementCount = m_mesh.drawElementsCount },
        };

        commandList->Begin(*m_renderpass);
        commandList->SetViewport(viewport);
        commandList->SetSicssor(scissor);
        commandList->Submit(drawCommand);

        auto drawData = ImGui::GetDrawData();
        m_imguiRenderer->RenderDrawData(drawData, *commandList);
        commandList->End();

        m_frameScheduler->End();
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
    RHI::ImageAttachment* m_swapchainAttachment;
};

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    return ApplicationBase::Entry<TriangleExample>();
}