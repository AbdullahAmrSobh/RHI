#include "Frame.hpp"
#include "Device.hpp"
#include "CommandList.hpp"
#include "Swapchain.hpp"
#include "Common.hpp"

#include <TL/Log.hpp>
#include <TL/Assert.hpp>

namespace RHI::WebGPU
{
    ResultCode IFrame::Init(IDevice* device)
    {
        m_device   = device;
        m_timeline = 0;
        return ResultCode::Success;
    }

    void IFrame::Shutdown()
    {
        m_acquiredSwapchains.clear();
    }

    void IFrame::CaptureNextFrame()
    {
        // WebGPU doesn't have Renderdoc support yet
    }

    void IFrame::Begin(TL::Span<SwapchainImageAcquireInfo> swapchainToAcquire)
    {
        m_acquiredSwapchains.clear();
        for (auto& acquireInfo : swapchainToAcquire)
        {
            auto swapchain = (ISwapchain*)(acquireInfo.swapchain);
            // WebGPU swapchain acquisition is handled differently
            m_acquiredSwapchains.push_back(acquireInfo.swapchain);
        }
    }

    void IFrame::End()
    {
        // Present swapchains
        for (auto* swapchain : m_acquiredSwapchains)
        {
            auto webgpuSwapchain = (ISwapchain*)swapchain;
            webgpuSwapchain->Present();
        }
    }

    CommandList* IFrame::CreateCommandList(const CommandListCreateInfo& createInfo)
    {
        auto handle = TL::construct<ICommandList>();
        auto result = handle->Init(m_device, createInfo);
        TL_ASSERT(IsSuccess(result));
        return handle;
    }

    void IFrame::DestroyCommandList(CommandList* commandList)
    {
        auto handle = (ICommandList*)commandList;
        handle->Shutdown();
        TL::destruct(handle);
    }

    uint64_t IFrame::QueueSubmit(QueueType queueType, const QueueSubmitInfo& submitInfo)
    {
        TL_ASSERT(queueType == QueueType::Graphics, "WebGPU only supports graphics queue currently");

        auto device = (IDevice*)m_device;
        TL::Vector<WGPUCommandBuffer> commandBuffers{device->GetTempAllocator()};
        commandBuffers.reserve(submitInfo.commandLists.size());

        for (auto* cmdList : submitInfo.commandLists)
        {
            auto webgpuCmdList = (ICommandList*)cmdList;
            commandBuffers.push_back(webgpuCmdList->m_cmdBuffer);
        }

        wgpuQueueSubmit(device->m_queue, commandBuffers.size(), commandBuffers.data());

        m_timeline.fetch_add(1, std::memory_order_relaxed);
        return m_timeline.load();
    }

    void IFrame::BufferWrite(Buffer* buffer, size_t offset, TL::Block block)
    {
        auto device = (IDevice*)m_device;
        auto webgpuBuffer = (IBuffer*)buffer;
        wgpuQueueWriteBuffer(device->m_queue, webgpuBuffer->buffer, offset, block.ptr, block.size);
    }

    void IFrame::ImageWrite(Image* image, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block)
    {
        TL_ASSERT(arrayLayer == 0, "Can't write to array layers");

        auto device = (IDevice*)m_device;
        auto webgpuImage = (IImage*)image;

        auto formatInfo = GetFormatInfo(webgpuImage->format);

        WGPUTexelCopyTextureInfo destination{
            .texture  = webgpuImage->texture,
            .mipLevel = mipLevel,
            .origin   = ConvertToOffset3D(offset),
            .aspect   = WGPUTextureAspect_All,
        };
        WGPUTexelCopyBufferLayout dataLayout{
            .offset       = 0,
            .bytesPerRow  = size.width * formatInfo.bytesPerBlock,
            .rowsPerImage = size.height,
        };
        auto writeSize = ConvertToExtent3D(size);

        wgpuQueueWriteTexture(device->m_queue, &destination, block.ptr, block.size, &dataLayout, &writeSize);
    }

    TL::IAllocator& IFrame::GetAllocator()
    {
        return m_arena;
    }

    uint64_t IFrame::GetTimelineValue() const
    {
        return m_timeline.load();
    }
} // namespace RHI::WebGPU

