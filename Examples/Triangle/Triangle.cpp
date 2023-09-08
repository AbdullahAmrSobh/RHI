#include <cassert>
#include <iostream>

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
        // create swapchain
        {
            RHI::SwapchainCreateInfo createInfo {};
            createInfo.win32Window.hwnd      = windowInfo.hwnd;
            createInfo.win32Window.hinstance = windowInfo.hinstance;
            createInfo.imageSize.width       = windowInfo.width;
            createInfo.imageSize.height      = windowInfo.height;
            createInfo.imageUsage            = RHI::ImageUsage::Color;
            createInfo.imageFormat           = RHI::Format::R8G8B8A8_UINT;
            createInfo.imageCount            = 2;

            m_swapchain = m_context->CreateSwapchain(createInfo);
        }

        // create resources pool
        {
            RHI::ResourcePoolCreateInfo createInfo {};
            createInfo.heapType            = RHI::MemoryType::GPULocal;
            createInfo.allocationAlgorithm = RHI::AllocationAlgorithm::Linear;
            createInfo.capacity            = 10 * RHI::AllocationSizeConstants::KB;
            createInfo.alignment           = alignof(uint64_t);

            m_resourcePool = m_context->CreateResourcePool(createInfo);
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

            RHI::BufferCreateInfo createInfo {};
            createInfo.usageFlags = RHI::BufferUsage::Vertex;
            createInfo.byteSize   = 6 * sizeof(float);
            {
                auto [vertexBuffer, result] = m_resourcePool->Allocate(createInfo);
                m_vertexBuffer              = vertexBuffer;
            }
            createInfo.usageFlags = RHI::BufferUsage::Index;
            createInfo.byteSize   = 6 * sizeof(uint32_t);
            {
                auto [indexBuffer, result] = m_resourcePool->Allocate(createInfo);
                m_indexBuffer              = indexBuffer;
            }

            RHI::DeviceMemoryPtr vertexBufferPtr = m_resourcePool->MapResource(m_vertexBuffer, 0, sizeof(float) * 6);
            assert(vertexBufferPtr != nullptr);
            memcpy(vertexBufferPtr, vertexData, sizeof(float) * 6);
            m_resourcePool->Unmap(m_vertexBuffer);

            RHI::DeviceMemoryPtr indexBufferPtr = m_resourcePool->MapResource(m_indexBuffer, 0, sizeof(uint32_t) * 6);
            assert(indexBufferPtr != nullptr);
            memcpy(indexBufferPtr, indexData, sizeof(uint32_t) * 6);
            m_resourcePool->Unmap(m_indexBuffer);
        }

        // create image resource
        {
            auto imageData = LoadImage("Resources/Images/image.png");

            RHI::ImageCreateInfo createInfo {};
            createInfo.usageFlags  = RHI::ImageUsage::ShaderResource;
            createInfo.type        = RHI::ImageType::Image2D;
            createInfo.size.width  = imageData.width;
            createInfo.size.height = imageData.height;
            createInfo.size.depth  = imageData.depth;
            createInfo.format      = RHI::Format::R8G8B8A8_UNORM;
            createInfo.mipLevels   = 1;
            createInfo.arrayCount  = 1;

            auto [image, result] = m_resourcePool->Allocate(createInfo);
            m_image              = image;

            RHI::DeviceMemoryPtr dataPtr = m_resourcePool->MapResource(m_image, 0, sizeof(uint32_t) * 6);
            assert(dataPtr != nullptr);
            memcpy(dataPtr, imageData.data.data(), imageData.data.size());
            m_resourcePool->Unmap(m_image);
        }

        // create shader bind group layout
        RHI::ShaderBindGroupLayout layout = {{RHI::ShaderBinding {RHI::ShaderBindingType::Image, RHI::ShaderBindingAccess::OnlyRead, 1}}};

        // create shader bind group
        m_shaderBindGroupAllocator = m_context->CreateShaderBindGroupAllocator();
        m_shaderBindGroup          = m_shaderBindGroupAllocator->AllocateShaderBindGroups(layout).front();

        // create pipeline
        {
            auto shaderCode   = ReadBinaryFile("./Shaders/triangle.spirv");
            auto shaderModule = m_context->CreateShaderModule(shaderCode);

            RHI::GraphicsPipelineCreateInfo createInfo {};
            createInfo.vertexShader.entryName                    = "VSMain";
            createInfo.vertexShader.shader                       = shaderModule.get();
            createInfo.vertexShader.stage                        = RHI::ShaderStage::Vertex;
            createInfo.pixelShader.entryName                     = "PSMain";
            createInfo.pixelShader.shader                        = shaderModule.get();
            createInfo.pixelShader.stage                         = RHI::ShaderStage::Pixel;
            createInfo.bindGroupLayouts                          = {layout};
            createInfo.renderTargetLayout.colorAttachmentsFormat = {RHI::Format::R8G8B8A8_UINT};
            createInfo.renderTargetLayout.depthAttachmentFormat  = RHI::Format::D32_FLOAT;

            m_pipelineState = m_context->CreateGraphicsPipeline(createInfo);
        }

        // create frame graph
        {
            // create frame scheduler
            m_frameScheduler = m_context->CreateFrameScheduler();

            RHI::PassCreateInfo createInfo {};
            createInfo.name = "RenderPass";
            createInfo.type = RHI::QueueType::Graphics;
            m_renderpass    = m_frameScheduler->CreatePass(createInfo);
        }
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

        // setup render pass.
        {
            m_renderpass->Begin();

            // setup attachments
            RHI::ImageCreateInfo createInfo {};
            createInfo.size.width = 800;
            createInfo.size.width = 600;
            createInfo.format     = RHI::Format::D32_FLOAT;

            RHI::ImageAttachmentUseInfo useInfo {};
            useInfo.clearValue.depth.depthValue = 1.0f;
            m_renderpass->CreateTransientImageResource("depth-attachment", createInfo, useInfo);

            useInfo.clearValue.color = {0.3f, 0.6f, 0.9f, 1.0f};
            m_renderpass->ImportImageResource("color-attachment", m_swapchain->GetImage(), useInfo);

            auto textureAttachment = m_renderpass->ImportImageResource("texture", m_image, useInfo);
            auto textureView       = m_frameScheduler->GetImageView(textureAttachment);

            // setup bind elements
            m_renderpass->End();

            RHI::ShaderBindGroupData shaderBindGroupData {};
            shaderBindGroupData.BindImages(0u, textureView);

            m_shaderBindGroupAllocator->Update(m_shaderBindGroup, shaderBindGroupData);
        }

        m_frameScheduler->Submit(*m_renderpass);

        m_frameScheduler->Compile();

        // Build command lists
        {
            auto& cmd = m_renderpass->BeginCommandList();

            // setup command list
            RHI::CommandDraw cmdDraw {
                .pipelineState {m_pipelineState},
                .shaderBindGroups {m_shaderBindGroup},
                .vertexBuffers {m_vertexBuffer},
                .indexBuffers {m_indexBuffer},
                .parameters {.elementCount = 6},
            };

            cmd.Submit(cmdDraw);

            m_renderpass->EndCommandList();
        }

        m_frameScheduler->End();
    }

private:
    std::unique_ptr<RHI::Swapchain> m_swapchain;

    std::unique_ptr<RHI::FrameScheduler> m_frameScheduler;

    std::unique_ptr<RHI::ResourcePool> m_resourcePool;

    std::unique_ptr<RHI::ShaderBindGroupAllocator> m_shaderBindGroupAllocator;

    RHI::Handle<RHI::GraphicsPipeline> m_pipelineState;

    RHI::Handle<RHI::Image> m_image;

    RHI::Handle<RHI::Buffer> m_vertexBuffer;

    RHI::Handle<RHI::Buffer> m_indexBuffer;

    RHI::Handle<RHI::ShaderBindGroup> m_shaderBindGroup;

    std::unique_ptr<RHI::Pass> m_renderpass;
};

EXAMPLE_ENTRY_POINT(TriangleExample)
