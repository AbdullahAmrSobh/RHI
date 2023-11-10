#include <Examples-Base/ExampleBase.hpp>
#include <RHI/RHI.hpp>

class TriangleExample final : public ExampleBase
{
public:
    TriangleExample()
        : ExampleBase("Hello, Triangle", 800, 600)
    {
    }

    void OnInit(WindowInfo windowInfo) override
    {
        // create resources pool
        {
            RHI::ResourcePoolCreateInfo createInfo{};
            createInfo.heapType            = RHI::MemoryType::CPU;
            createInfo.allocationAlgorithm = RHI::AllocationAlgorithm::Linear;
            createInfo.blockSize           = 10 * RHI::AllocationSizeConstants::KB;
            createInfo.minBlockAlignment   = alignof(uint64_t);

            m_resourcePool                 = m_context->CreateResourcePool(createInfo);
        }

        // create buffer resource
        {
            // clang-format off
            constexpr float vertexData[4 * 3] =
                {
                     1.0f, -1.0f, 0.0f,
                     1.0f,  1.0f, 0.0f,
                    -1.0f, -1.0f, 0.0f,
                    -1.0f,  1.0f, 0.0f,
                };

            constexpr uint32_t indexData[6] = { 0, 1, 2, 2, 3, 1};
            // clang-format on

            RHI::BufferCreateInfo createInfo{};
            createInfo.usageFlags = RHI::BufferUsage::Vertex;
            createInfo.byteSize   = 6 * sizeof(float);
            {
                auto [vertexBuffer, result] = m_resourcePool->Allocate(createInfo);
                RHI_ASSERT(result == RHI::ResultCode::Success);
                m_vertexBuffer = vertexBuffer;
            }
            createInfo.usageFlags = RHI::BufferUsage::Index;
            createInfo.byteSize   = 6 * sizeof(uint32_t);
            {
                auto [indexBuffer, result] = m_resourcePool->Allocate(createInfo);
                RHI_ASSERT(result == RHI::ResultCode::Success);
                m_indexBuffer = indexBuffer;
            }

            RHI::DeviceMemoryPtr vertexBufferPtr = m_context->MapResource(m_vertexBuffer);
            RHI_ASSERT(vertexBufferPtr != nullptr);
            memcpy(vertexBufferPtr, vertexData, sizeof(float) * 6);
            m_context->Unmap(m_vertexBuffer);

            RHI::DeviceMemoryPtr indexBufferPtr = m_context->MapResource(m_indexBuffer);
            RHI_ASSERT(indexBufferPtr != nullptr);
            memcpy(indexBufferPtr, indexData, sizeof(uint32_t) * 6);
            m_context->Unmap(m_indexBuffer);
        }

        // // create image resource
        // {
        //     auto imageData = LoadImage("Resources/Images/image.png");

        //     RHI::ImageCreateInfo createInfo{};
        //     createInfo.usageFlags = RHI::ImageUsage::ShaderResource;
        //     createInfo.type = RHI::ImageType::Image2D;
        //     createInfo.size.width = imageData.width;
        //     createInfo.size.height = imageData.height;
        //     createInfo.size.depth = imageData.depth;
        //     createInfo.format = RHI::Format::BGRA8_UNORM;
        //     createInfo.mipLevels = 1;
        //     createInfo.arrayCount = 1;

        //     auto [image, result] = m_resourcePool->Allocate(createInfo);
        //     m_image = image;

        //     RHI::DeviceMemoryPtr dataPtr = m_context->MapResource(m_image);
        //     RHI_ASSERT(dataPtr != nullptr);
        //     memcpy(dataPtr, imageData.data.data(), imageData.data.size());
        //     m_context->Unmap(m_image);
        // }

        // create shader bind group layout
        RHI::ShaderBindGroupLayout layout = { { RHI::ShaderBinding{ RHI::ShaderBindingType::Image, RHI::ShaderBindingAccess::OnlyRead, 1 } } };

        // create shader bind group
        m_shaderBindGroupAllocator = m_context->CreateShaderBindGroupAllocator();
        m_shaderBindGroup          = m_shaderBindGroupAllocator->AllocateShaderBindGroups(layout).front();

        // create pipeline
        {
            auto                        shaderCode = ReadBinaryFile("./Resources/Shaders/triangle.spv");

            RHI::ShaderModuleCreateInfo createInfo{};
            createInfo.code                              = shaderCode.data();
            createInfo.size                              = shaderCode.size();
            auto                            shaderModule = m_context->CreateShaderModule(createInfo);

            RHI::GraphicsPipelineCreateInfo psoCreateInfo{};
            psoCreateInfo.vertexShaderModule                 = shaderModule.get();
            psoCreateInfo.vertexShaderName                   = "VSMain";
            psoCreateInfo.pixelShaderModule                  = shaderModule.get();
            psoCreateInfo.pixelShaderName                    = "PSMain";
            psoCreateInfo.renderTargetLayout                 = { { RHI::Format::BGRA8_UNORM }, RHI::Format::D32, RHI::Format::Unkown };
            psoCreateInfo.depthStencilState.depthTestEnable  = true;
            psoCreateInfo.depthStencilState.depthWriteEnable = true;
            // psoCreateInfo.bindGroupLayouts                          = {layout};
            psoCreateInfo.renderTargetLayout.colorAttachmentsFormats = { RHI::Format::BGRA8_UNORM };
            psoCreateInfo.renderTargetLayout.depthAttachmentFormat   = RHI::Format::D32;
            psoCreateInfo.renderTargetLayout.colorAttachmentsFormats = { RHI::Format::BGRA8_UNORM };
            psoCreateInfo.renderTargetLayout.depthAttachmentFormat   = RHI::Format::D32;

            m_pipelineState                                          = m_context->CreateGraphicsPipeline(psoCreateInfo);
        }

        // create frame graph
        {
            RHI::PassCreateInfo createInfo{};
            createInfo.name = "RenderPass";
            createInfo.type = RHI::QueueType::Graphics;
            m_renderpass    = m_frameScheduler->CreatePass(createInfo);
        }

        {
            m_renderpass->Begin();

            // setup attachments
            RHI::ImageCreateInfo createInfo{};
            createInfo.size.width  = 800;
            createInfo.size.height = 600;
            createInfo.size.depth  = 1;
            createInfo.usageFlags  = RHI::ImageUsage::Depth;
            createInfo.format      = RHI::Format::D32;
            createInfo.type        = RHI::ImageType::Image2D;

            RHI::ImageAttachmentUseInfo useInfo{};

            useInfo.clearValue.depth.depthValue = 1.0f;
            useInfo.usage                       = RHI::AttachmentUsage::Depth;
            useInfo.subresource.imageAspects    = RHI::ImageAspect::Depth;
            m_renderpass->CreateTransientImageResource("depth-attachment", createInfo, useInfo);

            useInfo.clearValue.color         = { 0.3f, 0.6f, 0.9f, 1.0f };
            useInfo.usage                    = RHI::AttachmentUsage::RenderTarget;
            useInfo.subresource.imageAspects = RHI::ImageAspect::Color;
            m_renderpass->ImportImageResource("color-attachment", m_swapchain->GetImage(), useInfo);

            // auto textureAttachment = pass.ImportImageResource("texture", m_image, useInfo);

            // setup bind elements
            m_renderpass->End();

            // RHI::ShaderBindGroupData shaderBindGroupData{};
            // shaderBindGroupData.BindImages(0u, textureAttachment);
            // m_shaderBindGroupAllocator->Update(m_shaderBindGroup, shaderBindGroupData);

            m_frameScheduler->Submit(*m_renderpass);
        }

        m_frameScheduler->Compile();
    }

    void OnShutdown() override
    {
        m_context->Free(m_pipelineState);

        m_resourcePool->Free(m_image);

        m_resourcePool->Free(m_vertexBuffer);

        m_resourcePool->Free(m_indexBuffer);
    }

    void OnUpdate() override
    {
        m_frameScheduler->Begin();

        auto& cmd = m_renderpass->BeginCommandList();

        cmd.SetViewport({
            .width    = 800,
            .height   = 600,
            .minDepth = 0.0,
            .maxDepth = 1.0,
        });
        cmd.SetSicssor({
            .width  = 800,
            .height = 600,
        });
        cmd.Submit({
            .pipelineState{ m_pipelineState },
            .shaderBindGroups{ m_shaderBindGroup },
            .vertexBuffers{ m_vertexBuffer },
            .indexBuffers{ m_indexBuffer },
            .parameters{ .elementCount = 6 },
        });

        m_renderpass->EndCommandList();

        m_frameScheduler->End();
    }

private:
    std::unique_ptr<RHI::ResourcePool>             m_resourcePool;

    std::unique_ptr<RHI::ShaderBindGroupAllocator> m_shaderBindGroupAllocator;

    RHI::Handle<RHI::GraphicsPipeline>             m_pipelineState;

    RHI::Handle<RHI::Image>                        m_image;

    RHI::Handle<RHI::Buffer>                       m_vertexBuffer;

    RHI::Handle<RHI::Buffer>                       m_indexBuffer;

    RHI::Handle<RHI::ShaderBindGroup>              m_shaderBindGroup;

    std::unique_ptr<RHI::Pass>                     m_renderpass;
};

EXAMPLE_ENTRY_POINT(TriangleExample)
