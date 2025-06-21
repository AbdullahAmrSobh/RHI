#include "Frame.hpp"
#include "Device.hpp"
#include "CommandList.hpp"
#include "Resources.hpp"
#include "Swapchain.hpp"

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

    void IFrame::Begin(TL::Span<Swapchain* const> swapchains)
    {
        auto queue        = m_device->GetDeviceQueue(QueueType::Graphics);
        bool waitComplete = queue->WaitTimeline(m_prevTimeline);
        TL_ASSERT(waitComplete, "Failed to wait for current frame in flight");

        {
            uint64_t timeline;
            vkGetSemaphoreCounterValue(m_device->m_device, queue->GetTimelineHandle(), &timeline);
            m_device->m_destroyQueue->Flush(timeline);
        }

        m_tempAllocator.Collect();
        m_commandListAllocator->Reset();
        m_stagingPool->Reset();
        m_swapchains.clear();

        for (auto swapchain : swapchains)
            m_swapchains.push_back((ISwapchain*)swapchain);
    }

    uint64_t IFrame::End()
    {
        m_device->m_currentFrameIndex = (m_device->m_currentFrameIndex + 1) % (m_device->m_framesInFlight.size() - 1);
        m_prevTimeline                = m_timeline;
        return m_device->m_currentFrameIndex;
    }

    CommandList* IFrame::CreateCommandList(const CommandListCreateInfo& createInfo)
    {
        auto commandList = TL::ConstructFrom<ICommandList>(&m_tempAllocator);
        auto result      = commandList->Init(m_device, &m_commandListAllocator->m_queuePools[int(createInfo.queueType)], createInfo);
        TL_ASSERT(result == ResultCode::Success, "Failed to allocate command list for current frame");
        return commandList;
    }

    uint64_t IFrame::QueueSubmit(const QueueSubmitInfo& submitInfo)
    {
        auto queue = m_device->GetDeviceQueue(submitInfo.queueType);

        for (auto waitInfo : submitInfo.waitInfos)
        {
            if (auto waitQueue = m_device->GetDeviceQueue(waitInfo.queueType))
            {
                queue->AddWaitSemaphore(waitQueue->GetTimelineHandle(), waitInfo.timelineValue, ConvertPipelineStageFlags(waitInfo.waitStage));
            }
        }

        for (auto _swapchain : submitInfo.m_swapchainToAcquire)
        {
            auto swapchain = (ISwapchain*)_swapchain;
            // TODO: VkPipelineStageFlags can be deduced based on the swapchain image usage flags.
            queue->AddWaitSemaphore(swapchain->GetImageAcquiredSemaphore(), 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        }

        for (auto _swapchain : submitInfo.m_swapchainToSignal)
        {
            auto swapchain = (ISwapchain*)_swapchain;
            // TODO: VkPipelineStageFlags can be deduced based on the swapchain image usage flags.
            queue->AddSignalSemaphore(swapchain->GetImagePresentSemaphore(), 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }

        auto commandLists = TL::Span{(ICommandList**)submitInfo.commandLists.data(), submitInfo.commandLists.size()};
        return m_timeline = queue->Submit(commandLists, ConvertPipelineStageFlags(submitInfo.signalStage));
    }

    void IFrame::BufferWrite(Handle<Buffer> bufferHandle, size_t offset, TL::Block block)
    {
        auto buffer = m_device->Get(bufferHandle);

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

    void IFrame::ImageWrite(Handle<Image> imageHandle, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block)
    {
        auto [stagingBuffer, range] = AllocateStaging(block);
        TL_ASSERT(range.size >= block.size, "Unexpected error");

        // auto commandList = GetActiveTransferCommandList();

        auto queue = m_device->GetDeviceQueue(QueueType::Transfer);

        {
            auto                    image = m_device->m_imageOwner.Get(imageHandle);
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

        // TL_UNREACHABLE();
    }

    ////////////////////////////////////////////////////////////////
    /// Staging buffer allocator
    ////////////////////////////////////////////////////////////////

    constexpr static size_t MinStagingBufferAllocationSize = 64 * 1024 * 1024; // 64 mb

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
                    page.buffer, {offset, size}
                };
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
        auto buffer       = m_device->m_bufferOwner.Get(bufferHandle);

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
                          }
        };
    }

    StagingBufferBlock StagingBuffer::Allocate(TL::Block block)
    {
        auto stagingBlock = Allocate(block.size);
        auto buffer       = m_device->m_bufferOwner.Get(stagingBlock.buffer);
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

    ResultCode DeleteQueue::Init(IDevice* device)
    {
        m_device = device;
        return ResultCode::Success;
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