#include <Examples-Base/ApplicationBase.hpp>
#include <RHI/RHI.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct UniformBufferContent
{
    glm::mat4 viewProjection;
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

    struct Mesh
    {
        uint32_t drawElementsCount;

        RHI::Handle<RHI::Buffer> indexBuffer;
        RHI::Handle<RHI::Buffer> positionsBuffer;
        RHI::Handle<RHI::Buffer> normalsBuffer;
    };

    Mesh LoadScene(const char* path)
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

        auto indexBuffer = CreateBuffer<uint32_t>(indexBufferData, RHI::BufferUsage::Index);
        auto positionsBuffer = CreateBuffer<aiVector3D>({ mesh->mVertices, mesh->mNumVertices }, RHI::BufferUsage::Vertex);
        auto normalsBuffer = CreateBuffer<aiVector3D>({ mesh->mNormals, mesh->mNumVertices }, RHI::BufferUsage::Vertex);

        Mesh result{};
        result.drawElementsCount = indexBufferData.size();
        result.indexBuffer = indexBuffer;
        result.positionsBuffer = positionsBuffer;
        result.normalsBuffer = normalsBuffer;
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
        psoCreateInfo.inputAssemblerState.attributes = 
        {
            { .location = 0, .binding = 0, .format = RHI::Format::RGB32_FLOAT, .offset = 0, },
            { .location = 1, .binding = 1, .format = RHI::Format::RGB32_FLOAT, .offset = 0, },
        };
        psoCreateInfo.inputAssemblerState.bindings =
        {
            { .binding = 0, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex, },
            { .binding = 1, .stride = RHI::GetFormatByteSize(RHI::Format::RGB32_FLOAT), .stepRate = RHI::PipelineVertexInputRate::PerVertex, },
        };
        psoCreateInfo.vertexShaderModule = shaderModule.get();
        psoCreateInfo.vertexShaderName = "VSMain";
        psoCreateInfo.pixelShaderModule = shaderModule.get();
        psoCreateInfo.pixelShaderName = "PSMain";
        psoCreateInfo.topologyMode = RHI::PipelineTopologyMode::Triangles;
        psoCreateInfo.rasterizationState.cullMode = RHI::PipelineRasterizerStateCullMode::None;
        psoCreateInfo.renderTargetLayout = { { RHI::Format::BGRA8_UNORM }, RHI::Format::Unknown, RHI::Format::Unknown };
        psoCreateInfo.depthStencilState.depthTestEnable = true;
        psoCreateInfo.depthStencilState.depthWriteEnable = true;
        psoCreateInfo.depthStencilState.compareOperator = RHI::CompareOperator::Greater;
        psoCreateInfo.layout = m_pipelineLayout;
        psoCreateInfo.renderTargetLayout.colorAttachmentsFormats = { RHI::Format::BGRA8_UNORM };
        psoCreateInfo.renderTargetLayout.depthAttachmentFormat = RHI::Format::D32;
        psoCreateInfo.colorBlendState.blendStates = {
            {
                .blendEnable = true,
                .colorBlendOp = RHI::BlendEquation::Add,
                .srcColor = RHI::BlendFactor::One,
                .dstColor = RHI::BlendFactor::Zero,
                .alphaBlendOp = RHI::BlendEquation::Add,
                .srcAlpha = RHI::BlendFactor::One,
                .dstAlpha = RHI::BlendFactor::Zero,
            }
        };

        m_pipelineState = m_context->CreateGraphicsPipeline(psoCreateInfo);
    }

    void OnInit() override
    {
        {
            m_uniformData.viewProjection = m_camera.GetProjection() * m_camera.GetView();
        }

        // create resources pool
        {
            RHI::PoolCreateInfo createInfo{};
            createInfo.heapType = RHI::MemoryType::CPU;
            createInfo.allocationAlgorithm = RHI::AllocationAlgorithm::Linear;
            createInfo.blockSize = 64 * RHI::AllocationSizeConstants::MB;
            createInfo.minBlockAlignment = alignof(uint64_t);

            m_bufferPool = m_context->CreateBufferPool(createInfo);

            // create buffer resource
            m_mesh = LoadScene("./Resources/Meshes/StanfordBunny.obj");
            m_uniformBuffer = CreateBuffer<UniformBufferContent>(m_uniformData, RHI::BufferUsage::Uniform);
        }

        // create shader bind group layout
        m_bindGroupLayout = m_context->CreateBindGroupLayout({ RHI::ShaderBinding{ RHI::ShaderBindingType::Buffer, RHI::ShaderBindingAccess::OnlyRead, 1, RHI::ShaderStage::Vertex } });
        SetupPipelines(m_bindGroupLayout);
        // create shader bind group
        m_bindGroupAllocator = m_context->CreateBindGroupAllocator();
        m_bindGroup = m_bindGroupAllocator->AllocateBindGroups(m_bindGroupLayout).front();

        m_commandListAllocator = m_context->CreateCommandListAllocator(RHI::QueueType::Graphics);

        // create frame graph
        {
            RHI::PassCreateInfo createInfo{};
            createInfo.name = "RenderPass";
            createInfo.type = RHI::QueueType::Graphics;
            m_renderpass = m_frameScheduler->CreatePass(createInfo);
        }

        {
            m_renderpass->Begin();

            // setup attachments
            RHI::ImageAttachmentUseInfo useInfo{};
            useInfo.usage = RHI::AttachmentUsage::RenderTarget;
            useInfo.subresource.imageAspects = RHI::ImageAspect::Color;
            useInfo.loadStoreOperations.loadOperation = RHI::ImageLoadOperation::Discard;
            useInfo.loadStoreOperations.storeOperation = RHI::ImageStoreOperation::Store;
            useInfo.clearValue.color = { 0.3f, 0.6f, 0.9f, 1.0f };
            m_renderpass->ImportSwapchainImageResource("color-attachment", m_swapchain.get(), useInfo);

            RHI::ImageCreateInfo createInfo{};
            createInfo.usageFlags = RHI::ImageUsage::Depth;
            createInfo.size.width = m_windowWidth;
            createInfo.size.height = m_windowHeight;
            createInfo.size.depth = 1;
            createInfo.format = RHI::Format::D32;
            createInfo.type = RHI::ImageType::Image2D;

            useInfo.usage = RHI::AttachmentUsage::Depth;
            useInfo.subresource.imageAspects = RHI::ImageAspect::Depth;
            useInfo.clearValue.depth.depthValue = 1.0f;
            m_renderpass->CreateTransientImageResource("depth-attachment", createInfo, useInfo);

            // auto textureAttachment = m_renderpass->ImportImageResource("texture", m_image, useInfo);

            RHI::BufferAttachmentUseInfo bufferUseInfo{};
            bufferUseInfo.access = RHI::AttachmentAccess::Read;
            bufferUseInfo.usage = RHI::AttachmentUsage::ShaderResource;
            auto uniformBuffer = m_renderpass->ImportBufferResource("uniform-buffer", m_uniformBuffer, bufferUseInfo);

            // setup bind elements
            m_renderpass->End();

            RHI::BindGroupData BindGroupData{};
            BindGroupData.BindBuffers(0u, uniformBuffer);
            // BindGroupData.BindImages(1u, textureAttachment);
            m_bindGroupAllocator->Update(m_bindGroup, BindGroupData);

            m_frameScheduler->Submit(*m_renderpass);
        }
    }

    void OnShutdown() override
    {
        m_frameScheduler->WaitIdle(UINT64_MAX);

        m_bindGroupAllocator->Free(m_bindGroup);

        m_context->DestroyBindGroupLayout(m_bindGroupLayout);
        m_context->DestroyPipelineLayout(m_pipelineLayout);
        m_context->DestroyGraphicsPipeline(m_pipelineState);

        m_bufferPool->FreeBuffer(m_mesh.indexBuffer);
        m_bufferPool->FreeBuffer(m_mesh.positionsBuffer);
        m_bufferPool->FreeBuffer(m_uniformBuffer);
    }

    void OnUpdate(Timestep timestep) override
    {
        m_camera.Update(timestep);

        auto projection = m_camera.GetProjection() * m_camera.GetView();
        auto ptr = m_bufferPool->MapBuffer(m_uniformBuffer);
        memcpy(ptr, &projection, sizeof(glm::mat4));
        m_bufferPool->UnmapBuffer(m_uniformBuffer);

        (void)timestep;

        m_frameScheduler->Begin();

        m_commandListAllocator->Flush();

        auto cmd = m_commandListAllocator->Allocate();

        cmd->Begin(*m_renderpass);

        cmd->SetViewport({
            .width = float(m_windowWidth),
            .height = float(m_windowHeight),
            .minDepth = 0.0,
            .maxDepth = 1.0,
        });

        cmd->SetSicssor({
            .width = m_windowWidth,
            .height = m_windowHeight,
        });

        cmd->Submit({
            .pipelineState = m_pipelineState,
            .bindGroups = m_bindGroup,
            .vertexBuffers = { m_mesh.positionsBuffer, m_mesh.normalsBuffer },
            .indexBuffers = m_mesh.indexBuffer,
            .parameters = { .elementCount = m_mesh.drawElementsCount },
        });

        cmd->End();

        m_renderpass->Execute(cmd);

        m_frameScheduler->End();
    }

private:
    UniformBufferContent m_uniformData;

private:
    std::unique_ptr<RHI::BufferPool> m_bufferPool;

    std::unique_ptr<RHI::BindGroupAllocator> m_bindGroupAllocator;

    std::unique_ptr<RHI::CommandListAllocator> m_commandListAllocator;

    RHI::Handle<RHI::BindGroupLayout> m_bindGroupLayout;

    RHI::Handle<RHI::BindGroup> m_bindGroup;

    RHI::Handle<RHI::PipelineLayout> m_pipelineLayout;

    RHI::Handle<RHI::GraphicsPipeline> m_pipelineState;

    RHI::Handle<RHI::Buffer> m_uniformBuffer;

    Mesh m_mesh;

    std::unique_ptr<RHI::Pass> m_renderpass;
};

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    return ApplicationBase::Entry<TriangleExample>();
}