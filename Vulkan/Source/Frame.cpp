#include "Frame.hpp"
#include "Device.hpp"
#include "CommandList.hpp"
#include "Resources.hpp"
#include "Swapchain.hpp"

#include <TL/Literals.hpp>

namespace RHI::Vulkan
{
    ////////////////////////////////////////////////////////////////
    /// Frame
    ////////////////////////////////////////////////////////////////

    ResultCode IFrame::Init(IDevice* device)
    {
        m_device = device;

        m_commandListAllocator = TL::CreatePtr<CommandAllocator>();
        if (auto result = m_commandListAllocator->Init(m_device); IsError(result))
            return result;

        m_stagingPool = TL::CreatePtr<StagingBuffer>();
        if (auto result = m_stagingPool->Init(m_device); IsError(result))
            return result;

        VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = nullptr, .flags = 0};
        auto                  result = vkCreateSemaphore(m_device->m_device, &semaphoreCI, nullptr, &m_presentFrameSemaphore);

        return ResultCode::Success;
    }

    void IFrame::Shutdown()
    {
        m_stagingPool->Shutdown();
        m_commandListAllocator->Shutdown();
    }

    ICommandList* IFrame::GetActiveTransferCommandList()
    {
        return nullptr;
    }

    StagingBufferBlock IFrame::AllocateStaging(TL::Block block)
    {
        return m_stagingPool->Allocate(block);
    }

    void IFrame::CaptureNextFrame()
    {
        m_renderdocPendingCapture = true;
    }

    void IFrame::Begin(TL::Span<SwapchainImageAcquireInfo> swapchainToAcquire)
    {
        ZoneScoped;

        auto gfxQueue = m_device->GetDeviceQueue(QueueType::Graphics);

        {
            ZoneScopedN("Frame: Wait for frame ready");
            bool timeout = gfxQueue->Wait(m_timeline, UINT64_MAX);
            m_timeline   = gfxQueue->GetTimelineValue();
            TL_ASSERT(timeout != false);
        }

        {
            ZoneScopedN("Frame: cleanup temp allocations");
            m_stagingPool->Reset();
            m_commandListAllocator->Reset();
            m_acquiredSwapchains.clear();
            m_arena.Collect();
        }

        if (m_renderdocPendingCapture)
        {
            m_device->m_renderdoc->FrameStartCapture();
        }

        VulkanResult result;

        if (!swapchainToAcquire.empty())
        {
            ZoneScopedN("Frame: Acquire swapchain images");
            for (auto [_swapchain, pipelineStage] : swapchainToAcquire)
            {
                auto swapchain = (ISwapchain*)_swapchain;
                auto waitStage = ConvertPipelineStageFlags(pipelineStage);

                VkSemaphore imageAcquiredSemaphore{VK_NULL_HANDLE};
                result = swapchain->AcquireNextImage(imageAcquiredSemaphore);
                TL_ASSERT(result);
                m_device->GetDeviceQueue(QueueType::Graphics)->AddWaitSemaphore(imageAcquiredSemaphore, 0, waitStage);

                m_acquiredSwapchains.push_back({swapchain});
            }
        }
    }

    void IFrame::End()
    {
        auto gfxQueue      = m_device->GetDeviceQueue(QueueType::Graphics);
        auto cmpQueue      = m_device->GetDeviceQueue(QueueType::Compute);
        auto transferQueue = m_device->GetDeviceQueue(QueueType::Transfer);

        {
            ZoneScopedN("Frame: Present swapchain images");

            TL::Vector<VkSwapchainKHR> swapchains{m_arena};
            TL::Vector<uint32_t>       imageIndices{m_arena};
            TL_MAYBE_UNUSED TL::Vector<VkResult> results{m_arena};

            for (auto _swapchain : m_acquiredSwapchains)
            {
                auto swapchain = (ISwapchain*)_swapchain;
                swapchains.push_back(swapchain->GetHandle());
                imageIndices.push_back(swapchain->GetImageIndex());
            }
            if (!m_acquiredSwapchains.empty())
            {
                VkPresentInfoKHR presentInfo{
                    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .pNext              = nullptr,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores    = &m_presentFrameSemaphore,
                    .swapchainCount     = (uint32_t)swapchains.size(),
                    .pSwapchains        = swapchains.data(),
                    .pImageIndices      = imageIndices.data(),
                    .pResults           = results.data(),
                };
                VulkanResult result = vkQueuePresentKHR(gfxQueue->GetHandle(), &presentInfo);
                if (!result.IsSwapchainSuccess())
                {
                    TL_LOG_INFO("Swapchain present failed with error: {}", result.AsString());
                }
            }
        }

        if (m_renderdocPendingCapture)
        {
            m_device->m_renderdoc->FrameEndCapture();
            m_renderdocPendingCapture = false;
        }

        m_device->m_currentFrameIndex = (m_device->m_currentFrameIndex + 1) % (m_device->m_framesInFlight.size());
    }

    CommandList* IFrame::CreateCommandList(const CommandListCreateInfo& createInfo)
    {
        auto commandList = TL::ConstructFrom<ICommandList>(&m_arena);
        auto result      = commandList->Init(m_device, &m_commandListAllocator->m_queuePools[int(createInfo.queueType)], createInfo);
        TL_ASSERT(result == ResultCode::Success, "Failed to allocate command list for current frame");
        return commandList;
    }

    uint64_t IFrame::QueueSubmit(QueueType queueType, const QueueSubmitInfo& submitInfo)
    {
        ZoneScoped;

        auto queue = m_device->GetDeviceQueue(queueType);

        for (auto waitInfo : submitInfo.waitInfos)
        {
            if (auto waitQueue = m_device->GetDeviceQueue(waitInfo.queueType))
            {
                queue->AddWaitSemaphore(waitQueue->GetTimelineHandle(), waitInfo.timelineValue, ConvertPipelineStageFlags(waitInfo.waitStage));
            }
        }

        if (submitInfo.signalPresent == true)
        {
            queue->AddSignalSemaphore(m_presentFrameSemaphore, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
        }

        auto commandLists = TL::Span{(ICommandList**)submitInfo.commandLists.data(), submitInfo.commandLists.size()};
        m_timeline        = queue->Submit(commandLists, ConvertPipelineStageFlags(submitInfo.signalStage));
        m_timeline        = queue->GetTimelineValue();
        return queue->GetTimelineValue();
    }

    void IFrame::BufferWrite(Buffer* _buffer, size_t offset, TL::Block block)
    {
        auto buffer = (IBuffer*)_buffer;

        if (auto ptr = buffer->Map(m_device))
        {
            memcpy((char*)ptr + offset, block.ptr, block.size);
            buffer->Unmap(m_device);
            return;
        }

        auto [stagingBuffer, range] = AllocateStaging(block);
        TL_ASSERT(range.size >= block.size, "Unexpected error");

        auto commandList = GetActiveTransferCommandList();

        TL_UNREACHABLE();
    }

    void IFrame::ImageWrite(Image* imageHandle, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block)
    {
        auto [stagingBuffer, range] = AllocateStaging(block);
        TL_ASSERT(range.size >= block.size, "Unexpected error");

        // auto commandList = GetActiveTransferCommandList();
        auto queue = m_device->GetDeviceQueue(QueueType::Transfer);

        {
            auto                    image = (IImage*)imageHandle;
            VkImageSubresourceRange subresourceRange{
                .aspectMask     = image->SelectImageAspect(ImageAspect::All),
                .baseMipLevel   = mipLevel,
                .levelCount     = 1,
                .baseArrayLayer = arrayLayer,
                .layerCount     = 1,
            };

            auto _commandList = CreateCommandList({.queueType = QueueType::Transfer});
            auto copyCommand  = (ICommandList*)_commandList;

            copyCommand->Begin();
            copyCommand->AddPipelineBarriers({
                .imageBarriers = {{
                    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .pNext               = nullptr,
                    .srcStageMask        = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                    .srcAccessMask       = VK_ACCESS_2_NONE,
                    .dstStageMask        = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                    .dstAccessMask       = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image               = image->handle,
                    .subresourceRange    = subresourceRange,
                }},
            });
            copyCommand->CopyBufferToImage({
                .image       = imageHandle,
                .subresource = {
                    .imageAspects = ImageAspect::All,
                    .mipLevel     = mipLevel,
                    .arrayBase    = arrayLayer,
                    .arrayCount   = 1,
                },
                .imageSize     = size,
                .imageOffset   = offset,
                .buffer        = stagingBuffer,
                .bufferOffset  = range.offset,
                .bufferSize    = {},
                .bytesPerRow   = {},
                .bytesPerImage = {},
            });
            copyCommand->AddPipelineBarriers({
                .imageBarriers = {{
                    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .pNext               = nullptr,
                    .srcStageMask        = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                    .srcAccessMask       = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    .dstStageMask        = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                    .dstAccessMask       = VK_ACCESS_2_NONE,
                    .oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image               = image->handle,
                    .subresourceRange    = subresourceRange,
                }},
            });
            copyCommand->End();
            [[maybe_unused]] auto newTimeline = queue->Submit({copyCommand}, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
            vkDeviceWaitIdle(m_device->m_device);
        }
    }

    TL::IAllocator& IFrame::GetAllocator()
    {
        return m_arena;
    }

    uint64_t IFrame::GetTimelineValue() const
    {
        return m_timeline;
    }

    ////////////////////////////////////////////////////////////////
    /// Staging buffer allocator
    ////////////////////////////////////////////////////////////////

    constexpr static size_t MinStagingBufferAllocationSize = 64_mb;

    StagingBuffer::StagingBuffer()  = default;
    StagingBuffer::~StagingBuffer() = default;

    ResultCode StagingBuffer::Init(IDevice* device)
    {
        m_device = device;
        return ResultCode::Success;
    }

    void StagingBuffer::Shutdown()
    {
        for (auto& page : m_pages)
        {
            // m_device->UnmapBuffer(page.buffer);
            m_device->DestroyBuffer(page.buffer);
        }
    }

    StagingBufferBlock StagingBuffer::Allocate(size_t size)
    {
        for (auto& page : m_pages)
        {
            if (page.GetRemainingSize() >= size)
            {
                auto               offset       = page.offset;
                StagingBufferBlock stagingBlock = {
                    page.buffer, {offset, size}};
                page.offset += size;
                return stagingBlock;
            }
        }

        std::string      name = std::format("StagingBuffer-{}", m_pages.size());
        BufferCreateInfo stagingBufferCI{
            .name       = name.c_str(),
            .hostMapped = true,
            .usageFlags = BufferUsage::CopyDst | BufferUsage::CopySrc,
            // .byteSize   = std::max(size, static_cast<size_t>(64 * 1024 * 1024)), // 64 MB
            .byteSize   = size,
        };

        auto bufferHandle = m_device->CreateBuffer(stagingBufferCI);
        auto buffer       = (IBuffer*)(bufferHandle);

        m_pages.push_back(
            Page{
                .ptr    = buffer->Map(m_device),
                .buffer = bufferHandle,
                .offset = size,
                .size   = stagingBufferCI.byteSize,
            });

        return StagingBufferBlock{
            .buffer    = m_pages.back().buffer,
            .subregion = {
                0,
                size,
            }};
    }

    StagingBufferBlock StagingBuffer::Allocate(TL::Block block)
    {
        auto stagingBlock = Allocate(block.size);
        auto buffer       = (IBuffer*)(stagingBlock.buffer);
        auto ptr          = (char*)buffer->Map(m_device) + stagingBlock.subregion.offset;
        memcpy(ptr, block.ptr, block.size);
        return stagingBlock;
    }

    void StagingBuffer::Reset()
    {
        for (auto& page : m_pages)
        {
            page.offset = 0;
        }
    }

    ////////////////////////////////////////////////////////////////
    /// Release Queue
    ////////////////////////////////////////////////////////////////

    DeleteQueue::~DeleteQueue()
    {
        TL_ASSERT(m_allocation.empty());
        TL_ASSERT(m_buffer.empty());
        TL_ASSERT(m_bufferView.empty());
        TL_ASSERT(m_image.empty());
        TL_ASSERT(m_imageView.empty());
        TL_ASSERT(m_sampler.empty());
        TL_ASSERT(m_pipeline.empty());
        TL_ASSERT(m_descriptorPool.empty());
    }

    void DeleteQueue::Init(IDevice* device)
    {
        m_device = device;
    }

    void DeleteQueue::Shutdown()
    {
        Flush(UINT64_MAX);
    }

    void DeleteQueue::Flush(uint64_t timeline)
    {
        // NOTE: Order is important
        FlushQueue(*m_device, m_bufferView, timeline);
        FlushQueue(*m_device, m_buffer, timeline);
        FlushQueue(*m_device, m_imageView, timeline);
        FlushQueue(*m_device, m_image, timeline);
        FlushQueue(*m_device, m_allocation, timeline);
        FlushQueue(*m_device, m_sampler, timeline);
        FlushQueue(*m_device, m_pipeline, timeline);
        FlushQueue(*m_device, m_descriptorPool, timeline);
        FlushQueue(*m_device, m_swapchain, timeline);
        FlushQueue(*m_device, m_surface, timeline);
        FlushQueue(*m_device, m_semaphore, timeline);
    }
} // namespace RHI::Vulkan