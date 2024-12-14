#pragma once

#include <RHI/RenderGraphPass.hpp>

#include <vulkan/vulkan.h>

#include <TL/Containers.hpp>

// Remove
#include "Common.hpp"
#include "Image.hpp"

namespace RHI::Vulkan
{
    enum class BarrierSlot
    {
        Prilogue,
        Epilogue,
        Resolve,
        Count,
    };

    struct BarrierStage
    {
        VkPipelineStageFlags2 stageMask        = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2        accessMask       = VK_ACCESS_2_NONE;
        VkImageLayout         layout           = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t              queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    inline VkImageSubresourceRange GetAccessedSubresourceRange(const RenderGraphResourceTransition& accessedResource)
    {
        TL_ASSERT(accessedResource.resource->GetType() == RenderGraphResource::Type::Image);
        auto resource = (RenderGraphImage*)accessedResource.resource;

        return {
            .aspectMask     = ConvertImageAspect(GetFormatAspects(resource->GetFormat())),
            .baseMipLevel   = 0,
            .levelCount     = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount     = VK_REMAINING_ARRAY_LAYERS,
        };
    }

    inline BarrierStage GetBarrierStage(const RenderGraphResourceTransition* accessedResource)
    {
        if (accessedResource == nullptr) return {};

        switch (accessedResource->resource->GetType())
        {
        case RenderGraphResource::Type::Image:
            {
                auto usage  = accessedResource->asImage.usage;
                auto stage  = accessedResource->asImage.stage;
                auto access = accessedResource->asImage.access;
                return {
                    .stageMask        = ConvertPipelineStageFlags(stage),
                    .accessMask       = GetAccessFlags2(usage, access),
                    .layout           = GetImageLayout(usage, access, accessedResource->asImage.subresourceRange.imageAspects),
                    .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                };
            }
            break;
        case RenderGraphResource::Type::Buffer:
            {
                auto usage  = accessedResource->asBuffer.usage;
                auto stage  = accessedResource->asBuffer.stage;
                auto access = accessedResource->asBuffer.access;
                return {
                    .stageMask        = ConvertPipelineStageFlags(stage),
                    .accessMask       = GetAccessFlags2(usage, access),
                    .layout           = VK_IMAGE_LAYOUT_UNDEFINED,
                    .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                };
            }
            break;
        default:
            TL_UNREACHABLE();
            break;
        }
    }

    class CompiledPass final : public Pass
    {
    public:
        CompiledPass(const PassCreateInfo& createInfo, TL::IAllocator* allocator);

        TL::Span<const VkMemoryBarrier2>       GetMemoryBarriers(BarrierSlot slot) const;
        TL::Span<const VkImageMemoryBarrier2>  GetImageMemoryBarriers(BarrierSlot slot) const;
        TL::Span<const VkBufferMemoryBarrier2> GetBufferMemoryBarriers(BarrierSlot slot) const;
        void                                   PushPassBarrier(BarrierSlot slot, VkMemoryBarrier2&& barrier);
        void                                   PushPassBarrier(BarrierSlot slot, VkImageMemoryBarrier2&& barrier);
        void                                   PushPassBarrier(BarrierSlot slot, VkBufferMemoryBarrier2&& barrier);
        void                                   EmitBarriers(CommandList& _commnadList, BarrierSlot slot);

        void AddSwapchainPresentBarrier(Device& device, Swapchain& swapchain, RenderGraphResourceTransition& transition) override;

    private:
        TL::Vector<VkMemoryBarrier2, TL::IAllocator>       m_memoryBarriers[(int)BarrierSlot::Count];
        TL::Vector<VkImageMemoryBarrier2, TL::IAllocator>  m_imageMemoryBarriers[(int)BarrierSlot::Count];
        TL::Vector<VkBufferMemoryBarrier2, TL::IAllocator> m_bufferMemoryBarriers[(int)BarrierSlot::Count];
    };
} // namespace RHI::Vulkan
