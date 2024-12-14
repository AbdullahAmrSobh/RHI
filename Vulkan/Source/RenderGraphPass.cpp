#include "RenderGraphPass.hpp"

#include "CommandList.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

namespace RHI::Vulkan
{
    CompiledPass::CompiledPass(const PassCreateInfo& createInfo, TL::IAllocator* allocator)
        : Pass(createInfo, allocator)
        , m_memoryBarriers{
              TL::Vector<VkMemoryBarrier2, TL::IAllocator>(*allocator),
              TL::Vector<VkMemoryBarrier2, TL::IAllocator>(*allocator),
              TL::Vector<VkMemoryBarrier2, TL::IAllocator>(*allocator),
          }
        , m_imageMemoryBarriers{
              TL::Vector<VkImageMemoryBarrier2, TL::IAllocator>(*allocator),
              TL::Vector<VkImageMemoryBarrier2, TL::IAllocator>(*allocator),
              TL::Vector<VkImageMemoryBarrier2, TL::IAllocator>(*allocator),
          }
        , m_bufferMemoryBarriers{
              TL::Vector<VkBufferMemoryBarrier2, TL::IAllocator>(*allocator),
              TL::Vector<VkBufferMemoryBarrier2, TL::IAllocator>(*allocator),
              TL::Vector<VkBufferMemoryBarrier2, TL::IAllocator>(*allocator),
          }
    {
    }

    TL::Span<const VkMemoryBarrier2> CompiledPass::GetMemoryBarriers(BarrierSlot slot) const
    {
        return m_memoryBarriers[(int)slot];
    }

    TL::Span<const VkImageMemoryBarrier2> CompiledPass::GetImageMemoryBarriers(BarrierSlot slot) const
    {
        return m_imageMemoryBarriers[(int)slot];
    }

    TL::Span<const VkBufferMemoryBarrier2> CompiledPass::GetBufferMemoryBarriers(BarrierSlot slot) const
    {
        return m_bufferMemoryBarriers[(int)slot];
    }

    void CompiledPass::PushPassBarrier(BarrierSlot slot, VkMemoryBarrier2&& barrier)
    {
        m_memoryBarriers[(int)slot].emplace_back(barrier);
    }

    void CompiledPass::PushPassBarrier(BarrierSlot slot, VkImageMemoryBarrier2&& barrier)
    {
        m_imageMemoryBarriers[(int)slot].emplace_back(barrier);
    }

    void CompiledPass::PushPassBarrier(BarrierSlot slot, VkBufferMemoryBarrier2&& barrier)
    {
        m_bufferMemoryBarriers[(int)slot].emplace_back(barrier);
    }

    void CompiledPass::EmitBarriers(CommandList& _commnadList, BarrierSlot slot)
    {
        auto& commandList = (ICommandList&)_commnadList;
        commandList.AddPipelineBarriers({
            .memoryBarriers = m_memoryBarriers[(int)slot],
            .imageBarriers  = m_imageMemoryBarriers[(int)slot],
            .bufferBarriers = m_bufferMemoryBarriers[(int)slot],
        });
    }

    void CompiledPass::AddSwapchainPresentBarrier(Device& _device, Swapchain& _swapchain, RenderGraphResourceTransition& transition)
    {
        auto& device    = (IDevice&)_device;
        auto& swapchain = (ISwapchain&)_swapchain;
        auto  image     = device.m_imageOwner.Get(swapchain.GetImage());

        auto [srcStageMask, srcAccessMask, srcLayout, srcQfi] = GetBarrierStage(&transition);
        PushPassBarrier(
            BarrierSlot::Epilogue,
            {
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext               = nullptr,
                .srcStageMask        = srcStageMask,
                .srcAccessMask       = srcAccessMask,
                .dstStageMask        = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                .dstAccessMask       = VK_ACCESS_2_NONE,
                .oldLayout           = srcLayout,
                .newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image               = image->handle,
                .subresourceRange    = GetAccessedSubresourceRange(transition),
            });
    }

} // namespace RHI::Vulkan
