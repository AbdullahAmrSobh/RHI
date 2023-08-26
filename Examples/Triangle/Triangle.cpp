#include <cassert>
#include <iostream>

#include <Examples-Base/ExampleBase.hpp>
#include <RHI/RHI.hpp>

class TriangleExample final : public ExampleBase
{
public:
    void OnInit(const WindowInfo& windowInfo) override
    {
        // create swapchain
        {
            RHI::SwapchainCreateInfo createInfo {};
            createInfo.imageSize.width       = windowInfo.width;
            createInfo.imageSize.height      = windowInfo.height;
            createInfo.imageUsage            = RHI::ImageUsage::Color;
            createInfo.imageFormat           = RHI::Format::R8G8B8A8_UINT;
            createInfo.imageCount            = 2;
            createInfo.win32Window.hwnd      = windowInfo.hwnd;
            createInfo.win32Window.hinstance = windowInfo.hinstance;

            m_swapchain = m_context->CreateSwapchain(createInfo);
        }

        // create resources pool
        {
            RHI::ResourcePoolCreateInfo createInfo {};
            createInfo.heapType            = RHI::MemoryType::GPULocal;
            createInfo.allocationAlgorithm = RHI::AllocationAlgorithm::Linear;
            createInfo.capacity            = 10 * RHI::AllocationSizeConstants::MB;
            createInfo.alignment           = alignof(uint64_t);

            m_resourcePool = m_context->CreateResourcePool(createInfo);
        }

        // create buffer resource
        {
            constexpr float    vertexData[6] = {};
            constexpr uint32_t indexData[6]  = {};

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
            memcpy(dataPtr, imageData.GetPtr(), imageData.GetSize());
            m_resourcePool->Unmap(m_image);
        }

        // create shader bind group layout
        RHI::ShaderBindGroupLayout layout = {{RHI::ShaderBinding {RHI::ShaderBindingType::Image, RHI::ShaderBindingAccess::OnlyRead, 1}}};

        // create shader bind group allocator
        {
            
        }

        // create pipeline
        {
            auto shaderCode   = ReadBinaryFile("./Shaders/triangle.spirv");
            auto shaderModule = m_context->CreateShaderModule(shaderCode);

            RHI::GraphicsPipelineStateCreateInfo createInfo {};
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

        // create frame scheduler
        {
            m_frameScheduler = m_context->CreateFrameScheduler();
        }

        // Creates a render pass
        {
            auto setupAttachmentCallback = [=](RHI::FrameGraphBuilder& builder) {
                RHI::ImageAttachmentUseInfo useInfo {};
                useInfo.clearValue                         = {1.0f, 1.0f, 1.0f, 1.0f};
                useInfo.loadStoreOperations.loadOperation  = RHI::ImageLoadOperation::Load;
                useInfo.loadStoreOperations.storeOperation = RHI::ImageStoreOperation::Store;

                builder.UseRenderTarget("ColorOutput", useInfo);

                RHI::ImageAttachmentCreateInfo createInfo {};
                // builder.UseDepthTarget("DepthOutput", );
            };

            auto buildCommandListCallback = [=](RHI::CommandList& commandList) {
                RHI::CommandDraw cmdDraw {};
                cmdDraw.indexBuffers             = m_indexBuffer;
                cmdDraw.vertexBuffers            = {m_vertexBuffer};
                cmdDraw.pipelineState            = m_pipelineState;
                cmdDraw.shaderBindGroups         = {m_shaderBindGroup};
                cmdDraw.parameters.elementCount  = 6;
                cmdDraw.parameters.firstElement  = 0;
                cmdDraw.parameters.firstInstance = 0;
                cmdDraw.parameters.vertexOffset  = 0;
                cmdDraw.parameters.instanceCount = 1;

                commandList.Submit(cmdDraw);
            };

            m_renderPass = std::make_unique<RHI::PassProducerCallbacks>("RenderPass", RHI::PassQueue::Graphics, setupAttachmentCallback, buildCommandListCallback);
        }
    }

    void OnShutdown() override
    {
        m_resourcePool->Free(m_indexBuffer);

        m_resourcePool->Free(m_vertexBuffer);

        m_resourcePool->Free(m_image);
    }

    void OnUpdate() override
    {
        m_frameScheduler->Begin();

        m_frameScheduler->Submit(*m_renderPass);

        m_frameScheduler->End();
    }

private:
    std::unique_ptr<RHI::Swapchain> m_swapchain;

    std::unique_ptr<RHI::FrameScheduler> m_frameScheduler;

    std::unique_ptr<RHI::ResourcePool> m_resourcePool;

    std::unique_ptr<RHI::ShaderBindGroupAllocator> m_bindGroupAllocator;

    RHI::Handle<RHI::GraphicsPipeline> m_pipelineState;

    RHI::Handle<RHI::Image> m_image;

    RHI::Handle<RHI::Buffer> m_vertexBuffer;

    RHI::Handle<RHI::Buffer> m_indexBuffer;

    RHI::Handle<RHI::ShaderBindGroup> m_shaderBindGroup;

    std::unique_ptr<RHI::PassProducerCallbacks> m_renderPass;
};

EXAMPLE_ENTRY_POINT(TriangleExample)
