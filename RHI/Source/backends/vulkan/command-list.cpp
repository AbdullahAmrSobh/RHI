#include "command-list.h"
#include "common.h"
#include "device.h"
#include "resources.h"

#include <tracy/TracyVulkan.hpp>

#include <TL/Containers/InlineVector.hpp>

#include <algorithm>
#include <cstring>
#include <optional>

namespace RHI::Vulkan
{
    inline static VkAccessFlags2 GetAccessFlags2(ImageUsage usage, TL::Flags<Access> access)
    {
        VkAccessFlags2 result = VK_ACCESS_2_NONE;
        switch (usage)
        {
        case ImageUsage::ShaderResource:
            if (access & Access::Read) result |= VK_ACCESS_2_SHADER_READ_BIT;
            break;
        case ImageUsage::StorageResource:
            if (access & Access::Read) result |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
            break;
        case ImageUsage::CopySrc:
        case ImageUsage::CopyDst:
            if (access & Access::Read) result |= VK_ACCESS_2_TRANSFER_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        case ImageUsage::Color:
            if (access & Access::Read) result |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case ImageUsage::Depth:
        case ImageUsage::Stencil:
        case ImageUsage::DepthStencil:
            if (access & Access::Read) result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case ImageUsage::Present:
            if (access & Access::Read) result |= VK_ACCESS_2_NONE;
            if (access & Access::Write) result |= VK_ACCESS_2_NONE;
            break;
        default: break;
        };
        return result;
    }

    inline static VkAccessFlags2 GetAccessFlags2(BufferUsage usage, TL::Flags<Access> access)
    {
        VkAccessFlags2 result = VK_ACCESS_2_NONE;
        switch (usage)
        {
        case BufferUsage::Vertex:
            result |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
            TL_ASSERT((access & Access::Write) == Access::None, "BufferUsage::Vertex can't have write access");
            break;
        case BufferUsage::Index:
            result |= VK_ACCESS_2_INDEX_READ_BIT;
            TL_ASSERT((access & Access::Write) == Access::None, "BufferUsage::Index can't have write access");
            break;
        case BufferUsage::Uniform:
            if (access & Access::Read) result |= VK_ACCESS_2_UNIFORM_READ_BIT;
            TL_ASSERT((access & Access::Write) == Access::None, "BufferUsage::Uniform can't have write access");
            break;
        case BufferUsage::Storage:
            if (access & Access::Read) result |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
            break;
        case BufferUsage::CopySrc:
        case BufferUsage::CopyDst:
            if (access & Access::Read) result |= VK_ACCESS_2_TRANSFER_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        case BufferUsage::Indirect:
            result |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        default: break;
        };
        return result;
    }

    inline static VkImageLayout GetImageLayout(ImageUsage usage, TL::Flags<Access> access, TL::Flags<ImageAspect> aspect)
    {
        bool isReadOnly = access == Access::Read;
        switch (usage)
        {
        case ImageUsage::None: return VK_IMAGE_LAYOUT_UNDEFINED;
        case ImageUsage::ShaderResource:
            {
                if (aspect & ImageAspect::Color)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
                }
                else if (aspect & ImageAspect::DepthStencil)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
                else if (aspect & ImageAspect::Depth)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                }
                else if (aspect & ImageAspect::Stencil)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                }

                TL_UNREACHABLE();
                return VK_IMAGE_LAYOUT_GENERAL;
            }
        case ImageUsage::StorageResource: return VK_IMAGE_LAYOUT_GENERAL;
        case ImageUsage::Color: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // TODO: use <DEPTH/STENCIL>_READ_ONLY_OPTIMAL
        case ImageUsage::Depth: return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        case ImageUsage::Stencil: return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageUsage::DepthStencil: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageUsage::CopySrc: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ImageUsage::CopyDst: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ImageUsage::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        TL_UNREACHABLE();
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }

    inline static VkAttachmentLoadOp ConvertLoadOp(LoadOperation op)
    {
        switch (op)
        {
        case LoadOperation::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case LoadOperation::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
        case LoadOperation::Discard: return VK_ATTACHMENT_LOAD_OP_CLEAR;
        }
        TL_UNREACHABLE();
        return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
    }

    inline static VkAttachmentStoreOp ConvertStoreOp(StoreOperation op)
    {
        switch (op)
        {
        case StoreOperation::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case StoreOperation::Store: return VK_ATTACHMENT_STORE_OP_STORE;
        }
        TL_UNREACHABLE();
        return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
    }

    inline static VkResolveModeFlagBits ConvertResolveMode(ResolveMode resolveMode)
    {
        switch (resolveMode)
        {
        case ResolveMode::None: return VK_RESOLVE_MODE_NONE;
        case ResolveMode::Min: return VK_RESOLVE_MODE_MIN_BIT;
        case ResolveMode::Max: return VK_RESOLVE_MODE_MAX_BIT;
        case ResolveMode::Avg: return VK_RESOLVE_MODE_AVERAGE_BIT;
        }
        TL_UNREACHABLE();
        return VK_RESOLVE_MODE_FLAG_BITS_MAX_ENUM;
    }

    inline static VkPipelineBindPoint ConvertBindPoint(BindPoint bp)
    {
        switch (bp)
        {
        case BindPoint::Graphics: return VK_PIPELINE_BIND_POINT_GRAPHICS;
        case BindPoint::Compute: return VK_PIPELINE_BIND_POINT_COMPUTE;
        case BindPoint::RayTracing: return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
        }
        TL_UNREACHABLE();
        return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }

    struct BarrierStage
    {
        VkPipelineStageFlags2 stageMask = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 accessMask = VK_ACCESS_2_NONE;
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    inline static VkImageSubresourceLayers ConvertSubresourceLayers(const ImageCopyInfo& copyInfo, Format format)
    {
        return VkImageSubresourceLayers{
            .aspectMask = ConvertImageAspect(copyInfo.aspect, format),
            .mipLevel = copyInfo.mipLevel,
            .baseArrayLayer = copyInfo.arrayLayer,
            .layerCount = 1,
        };
    }

    inline static BarrierStage ConvertBarrierState(const BarrierState& barrierState)
    {
        // Global memory barrier: no resource usage is known here, so use the generic memory
        // access flags, which are valid against any pipeline stage.
        VkAccessFlags2 accessMask = VK_ACCESS_2_NONE;
        if (barrierState.access & Access::Read) accessMask |= VK_ACCESS_2_MEMORY_READ_BIT;
        if (barrierState.access & Access::Write) accessMask |= VK_ACCESS_2_MEMORY_WRITE_BIT;
        return {
            .stageMask = ConvertPipelineStageFlags(barrierState.stage),
            .accessMask = accessMask,
            .layout = VK_IMAGE_LAYOUT_UNDEFINED,
            .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
    }

    inline static BarrierStage ConvertBarrierState(const ImageBarrierState& barrierState)
    {
        return {
            .stageMask = ConvertPipelineStageFlags(barrierState.stage),
            .accessMask = GetAccessFlags2(barrierState.usage, barrierState.access),
            .layout = GetImageLayout(barrierState.usage, barrierState.access, ImageAspect::All),
            .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
    }

    inline static BarrierStage ConvertBarrierState(const BufferBarrierState& barrierState)
    {
        return {
            .stageMask = ConvertPipelineStageFlags(barrierState.stage),
            .accessMask = GetAccessFlags2(barrierState.usage, barrierState.access),
            .layout = VK_IMAGE_LAYOUT_UNDEFINED,
            .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
    }

    inline static VkStridedDeviceAddressRegionKHR ConvertStridedDeviceAddressRegion(const StridedDeviceAddressRegion& r)
    {
        // NOTE: DispatchRaysInfo carries no SBT buffer handle, so `offset` is the region's absolute
        // device address (caller must add the SBT buffer's base address), not a buffer-relative offset.
        return VkStridedDeviceAddressRegionKHR{
            .deviceAddress = (VkDeviceAddress)r.offset,
            .stride = r.stride,
            .size = r.size,
        };
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// ICommandPool
    //////////////////////////////////////////////////////////////////////////////////////////

    ResultCode ICommandPool::Init(IDevice* device, const CommandPoolCreateInfo& createInfo)
    {
        this->device = device;
        IQueue* queue = device->getQueue(createInfo.queue);
        this->queue = queue;

        VkCommandPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queue->m_familyIndex,
        };

        VulkanResult result = vkCreateCommandPool(device->m_device, &poolInfo, nullptr, &commandPool);
        if (result && createInfo.name)
            device->SetDebugName(VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)commandPool, createInfo.name);
        return result;
    }

    void ICommandPool::Shutdown(IDevice* device)
    {
        vkDestroyCommandPool(device->m_device, commandPool, nullptr);
    }

    void commandPoolReset(ICommandPool* self)
    {
        vkResetCommandPool(self->device->m_device, self->commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        self->arena.reset();
    }

    CommandList* commandPoolAllocate(ICommandPool* self)
    {
        TL_ASSERT(self->commandList.size() + 1 <= Limits::CommandListsPerPool, "Command-pool command-list capacity exceeded");
        VkCommandBufferAllocateInfo allocateInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            nullptr,
            self->commandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1,
        };
        ICommandList* commandList = TL::constructFrom<ICommandList>(&self->arena);
        commandList->device = self->device;
        commandList->commandPool = self;
        VK_CHECK(vkAllocateCommandBuffers(self->device->m_device, &allocateInfo, &commandList->commandBuffer));
        return commandList;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    void cmdBegin(ICommandList* self)
    {
        TL_ASSERT(self->tracyScopes.empty(), "Command list began with unclosed debug markers");

        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr,
        };
        vkBeginCommandBuffer(self->commandBuffer, &beginInfo);
#if defined(TRACY_ENABLE)
        if (auto tracyContext = static_cast<TracyVkCtx>(self->commandPool->queue->m_tracyContext))
        {
            const std::lock_guard lock(self->commandPool->queue->m_tracyCollectMutex);
            TracyVkCollect(tracyContext, self->commandBuffer);
        }
#endif
    }

    void cmdEnd(ICommandList* self)
    {

        TL_ASSERT(self->tracyScopes.empty(), "Command list ended with unclosed debug markers");
#if defined(TRACY_ENABLE)
        while (!self->tracyScopes.empty())
        {
            static_cast<tracy::VkCtxScope*>(self->tracyScopes.back())->~VkCtxScope();
            self->tracyScopes.pop_back();
        }
#endif

        vkEndCommandBuffer(self->commandBuffer);
    }

    void cmdPushDebugMarker(ICommandList* self, TL_MAYBE_UNUSED const char* name, TL_MAYBE_UNUSED uint32_t bgra)
    {

#if defined(TRACY_ENABLE)
        if (auto tracyContext = static_cast<TracyVkCtx>(self->commandPool->queue->m_tracyContext))
        {
            TL_ASSERT(self->tracyScopes.size() + 1 <= self->tracyScopes.capacity(), "Command-list debug-marker nesting exceeded Tracy scope capacity");
            auto* scope = TL::constructFrom<tracy::VkCtxScope>(
                &self->commandPool->arena,
                tracyContext,
                __LINE__,
                __FILE__, sizeof(__FILE__) - 1,
                __func__, std::strlen(__func__),
                name, std::strlen(name),
                self->commandBuffer,
                true);
            self->tracyScopes.push_back(scope);
        }
#endif

#if RHI_DEBUG
        if (auto fn = vkCmdBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT info = MakeDebugLabel(name, bgra);
            fn(self->commandBuffer, &info);
        }
#endif
    }

    void cmdPopDebugMarker(ICommandList* self)
    {
#if defined(TRACY_ENABLE)
        if (self->commandPool->queue->m_tracyContext)
        {
            TL_ASSERT(!self->tracyScopes.empty(), "Debug-marker pop has no matching push");
            if (!self->tracyScopes.empty())
            {
                static_cast<tracy::VkCtxScope*>(self->tracyScopes.back())->~VkCtxScope();
                self->tracyScopes.pop_back();
            }
        }
#endif
#if RHI_DEBUG
        if (auto fn = vkCmdEndDebugUtilsLabelEXT)
        {
            fn(self->commandBuffer);
        }
#endif
    }

    void cmdInsertDebugMarker(ICommandList* self, TL_MAYBE_UNUSED const char* name, TL_MAYBE_UNUSED uint32_t bgra)
    {

#if RHI_DEBUG
        if (auto fn = vkCmdInsertDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT info = MakeDebugLabel(name, bgra);
            fn(self->commandBuffer, &info);
        }
#endif
    }

    void cmdAddPipelineBarrier(ICommandList* self, TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers)
    {

        if (barriers.empty() && imageBarriers.empty() && bufferBarriers.empty())
            return;

        TL_ASSERT(barriers.size() <= Limits::BarrierBatch, "Too many memory barriers in one command");
        TL_ASSERT(bufferBarriers.size() <= Limits::BarrierBatch, "Too many buffer barriers in one command");
        TL_ASSERT(imageBarriers.size() <= Limits::BarrierBatch, "Too many image barriers in one command");
        TL::InlineVector<VkMemoryBarrier2, Limits::BarrierBatch> vmemoryBarriers;
        TL::InlineVector<VkBufferMemoryBarrier2, Limits::BarrierBatch> vbufferBarriers;
        TL::InlineVector<VkImageMemoryBarrier2, Limits::BarrierBatch> vimageBarriers;

        for (auto barrier : barriers)
        {
            auto [srcStageMask, srcAccessMask, srcLayout, srcQueueFamilyIndex] = ConvertBarrierState(barrier.srcState);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQueueFamilyIndex] = ConvertBarrierState(barrier.dstState);

            vmemoryBarriers.push_back({
                .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = srcStageMask,
                .srcAccessMask = srcAccessMask,
                .dstStageMask = dstStageMask,
                .dstAccessMask = dstAccessMask,
            });
        }

        for (auto imageBarrier : imageBarriers)
        {
            auto image = (IImage*)(imageBarrier.image);

            auto [srcStageMask, srcAccessMask, srcLayout, srcQueueFamilyIndex] = ConvertBarrierState(imageBarrier.srcState);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQueueFamilyIndex] = ConvertBarrierState(imageBarrier.dstState);

            // A default (All()) subresource means "the whole image"; resolve it to the image's
            // actual range so the barrier carries real mip/array counts.
            ImageSubresourceRange subresource = imageBarrier.subresource;
            if (subresource == ImageSubresourceRange::All())
                subresource = image->subresources;

            vimageBarriers.push_back({
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = srcStageMask,
                .srcAccessMask = srcAccessMask,
                .dstStageMask = dstStageMask,
                .dstAccessMask = dstAccessMask,
                .oldLayout = srcLayout,
                .newLayout = dstLayout,
                .srcQueueFamilyIndex = srcQueueFamilyIndex,
                .dstQueueFamilyIndex = dstQueueFamilyIndex,
                .image = image->handle,
                .subresourceRange = ConvertSubresourceRange(subresource, image->format),
            });
        }

        for (auto bufferBarrier : bufferBarriers)
        {
            auto buffer = (IBuffer*)(bufferBarrier.buffer);

            auto [srcStageMask, srcAccessMask, srcLayout, srcQueueFamilyIndex] = ConvertBarrierState(bufferBarrier.srcState);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQueueFamilyIndex] = ConvertBarrierState(bufferBarrier.dstState);

            vbufferBarriers.push_back({
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = srcStageMask,
                .srcAccessMask = srcAccessMask,
                .dstStageMask = dstStageMask,
                .dstAccessMask = dstAccessMask,
                .srcQueueFamilyIndex = srcQueueFamilyIndex,
                .dstQueueFamilyIndex = dstQueueFamilyIndex,
                .buffer = buffer->handle,
                .offset = bufferBarrier.subregion.offset,
                .size = VK_WHOLE_SIZE, // bufferBarrier.subregion.size,
            });
        }

        VkDependencyInfo dependencyInfo =
            {
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .pNext = nullptr,
                .dependencyFlags = 0,
                .memoryBarrierCount = uint32_t(vmemoryBarriers.size()),
                .pMemoryBarriers = vmemoryBarriers.data(),
                .bufferMemoryBarrierCount = uint32_t(vbufferBarriers.size()),
                .pBufferMemoryBarriers = vbufferBarriers.data(),
                .imageMemoryBarrierCount = uint32_t(vimageBarriers.size()),
                .pImageMemoryBarriers = vimageBarriers.data(),
            };
        vkCmdPipelineBarrier2(self->commandBuffer, &dependencyInfo);
    }

    void cmdBeginRenderPass(ICommandList* self, const RenderPassBeginInfo& beginInfo)
    {

        TL_ASSERT(beginInfo.colorAttachments.size() <= Limits::ColorAttachments, "Too many color attachments");
        TL::InlineVector<VkRenderingAttachmentInfo, Limits::ColorAttachments> colorAttachments;
        std::optional<VkRenderingAttachmentInfo> depthAttachment{};
        std::optional<VkRenderingAttachmentInfo> stencilAttachment{};

        for (const auto& colorAttachment : beginInfo.colorAttachments)
        {
            auto colorImage = (IImage*)(colorAttachment.view);
            auto resolveView = colorAttachment.resolveView ? (IImage*)(colorAttachment.resolveView) : nullptr;
            colorAttachments.push_back({
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext = nullptr,
                .imageView = colorImage->viewHandle,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = ConvertResolveMode(colorAttachment.resolveMode),
                .resolveImageView = resolveView ? resolveView->viewHandle : VK_NULL_HANDLE,
                .resolveImageLayout = resolveView ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp = ConvertLoadOp(colorAttachment.loadOp),
                .storeOp = ConvertStoreOp(colorAttachment.storeOp),
                .clearValue = {colorAttachment.clearValue.f32.r, colorAttachment.clearValue.f32.g, colorAttachment.clearValue.f32.b, colorAttachment.clearValue.f32.a},
            });
        }

        if (beginInfo.depthStencilAttachment.view)
        {
            auto image = (IImage*)(beginInfo.depthStencilAttachment.view);

            if (image->subresources.imageAspects & ImageAspect::Depth)
            {
                depthAttachment = VkRenderingAttachmentInfo{
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext = nullptr,
                    .imageView = image->viewHandle,
                    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                    .resolveMode = VK_RESOLVE_MODE_NONE,
                    .resolveImageView = VK_NULL_HANDLE,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .loadOp = ConvertLoadOp(beginInfo.depthStencilAttachment.depthLoadOp),
                    .storeOp = ConvertStoreOp(beginInfo.depthStencilAttachment.depthStoreOp),
                    .clearValue = {.depthStencil = {.depth = beginInfo.depthStencilAttachment.clearValue.depthValue, .stencil = beginInfo.depthStencilAttachment.clearValue.stencilValue}},
                };
            }

            if (image->subresources.imageAspects & ImageAspect::Stencil)
            {
                stencilAttachment = VkRenderingAttachmentInfo{
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext = nullptr,
                    .imageView = image->viewHandle,
                    .imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                    .resolveMode = VK_RESOLVE_MODE_NONE,
                    .resolveImageView = VK_NULL_HANDLE,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .loadOp = ConvertLoadOp(beginInfo.depthStencilAttachment.stencilLoadOp),
                    .storeOp = ConvertStoreOp(beginInfo.depthStencilAttachment.stencilStoreOp),
                    .clearValue = {.depthStencil = {.depth = beginInfo.depthStencilAttachment.clearValue.depthValue, .stencil = beginInfo.depthStencilAttachment.clearValue.stencilValue}},
                };
            }
            if ((image->subresources.imageAspects & ImageAspect::DepthStencil) == ImageAspect::DepthStencil)
            {
                depthAttachment->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                stencilAttachment->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
        }

        auto [offsetX, offsetY] = beginInfo.offset;
        auto [width, height] = beginInfo.size;
        VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderArea = {.offset = {offsetX, offsetY}, .extent = {width, height}},
            .layerCount = 1,
            .viewMask = 0,
            .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
            .pColorAttachments = colorAttachments.data(),
            .pDepthAttachment = depthAttachment.has_value() ? &depthAttachment.value() : nullptr,
            .pStencilAttachment = stencilAttachment.has_value() ? &stencilAttachment.value() : nullptr,
        };
        vkCmdBeginRendering(self->commandBuffer, &renderingInfo);
    }

    void cmdEndRenderPass(ICommandList* self)
    {
        vkCmdEndRendering(self->commandBuffer);
    }

    void cmdBeginComputePass(ICommandList* self, TL_MAYBE_UNUSED const ComputePassBeginInfo& beginInfo)
    {
        // No-Op
    }

    void cmdEndComputePass(ICommandList* self)
    {
        // No-Op
    }

    void cmdBeginConditionalCommands(ICommandList* self, const BufferBindingInfo& conditionBuffer, bool inverted)
    {

        auto buffer = (IBuffer*)(conditionBuffer.buffer);

        VkConditionalRenderingBeginInfoEXT beginInfo{
            .sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT,
            .pNext = nullptr,
            .buffer = buffer->handle,
            .offset = conditionBuffer.offset,
            .flags = inverted ? VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT : 0u,
        };
        vkCmdBeginConditionalRenderingEXT(self->commandBuffer, &beginInfo);
    }

    void cmdEndConditionalCommands(ICommandList* self)
    {

        vkCmdEndConditionalRenderingEXT(self->commandBuffer);
    }

    void cmdExecute(ICommandList* self, TL::Span<const CommandList*> commandLists)
    {

        TL_ASSERT(commandLists.size() <= Limits::CommandBatch, "Too many secondary command buffers in one Execute call");
        TL::InlineVector<VkCommandBuffer, Limits::CommandBatch> commandBuffers;

        for (const auto* commandList : commandLists)
        {
            auto vkCmdList = (const ICommandList*)commandList;
            commandBuffers.push_back(vkCmdList->commandBuffer);
        }

        vkCmdExecuteCommands(self->commandBuffer, commandBuffers.size(), commandBuffers.data());
    }

    void cmdBindPipelineLayout(ICommandList* self, BindPoint bindPoint, const PipelineLayout* pipelineLayout)
    {

        self->pipelineLayout = (PipelineLayout*)pipelineLayout;
        self->pipelineBindPoint = bindPoint == BindPoint::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    void cmdSetPushConstants(ICommandList* self, TL_MAYBE_UNUSED BindPoint bindPoint, uint32_t offset, TL::Block content)
    {

        IPipelineLayout* pipelineLayout = (IPipelineLayout*)self->pipelineLayout;

        vkCmdPushConstants(self->commandBuffer, pipelineLayout->handle, pipelineLayout->pushConstantStages, offset, (uint32_t)content.size, content.ptr);
    }

    void cmdPushBindGroup(ICommandList* self, BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos)
    {

        IPipelineLayout*  pipelineLayout = (IPipelineLayout*)self->pipelineLayout;
        IBindGroupLayout* groupLayout    = pipelineLayout->bindGroupLayouts[firstGroup];

        // Descriptors are pushed in fixed-size batches out of stack storage, one push per batch.
        for (const BindGroupUpdateInfo& updateInfo : updateInfos)
        {
            for (const BindGroupBuffersUpdateInfo& update : updateInfo.buffers)
            {
                const VkDescriptorType descriptorType = ConvertDescriptorType(groupLayout->GetBinding(update.dstBinding).type);
                for (size_t first = 0; first < update.buffers.size(); first += Limits::DescriptorBatch)
                {
                    const size_t           count = (std::min)(Limits::DescriptorBatch, update.buffers.size() - first);
                    VkDescriptorBufferInfo infos[Limits::DescriptorBatch];
                    for (size_t i = 0; i < count; ++i)
                    {
                        const BufferBindingInfo& binding = update.buffers[first + i];
                        const auto*              buffer  = static_cast<const IBuffer*>(binding.buffer);
                        infos[i] = {
                            .buffer = buffer->handle,
                            .offset = binding.offset,
                            .range  = binding.range == RemainingSize ? VK_WHOLE_SIZE : binding.range,
                        };
                    }
                    const VkWriteDescriptorSet write{
                        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstBinding      = update.dstBinding,
                        .dstArrayElement = update.dstArrayElement + static_cast<uint32_t>(first),
                        .descriptorCount = static_cast<uint32_t>(count),
                        .descriptorType  = descriptorType,
                        .pBufferInfo     = infos,
                    };
                    const VkPushDescriptorSetInfo pushInfo{
                        .sType                = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO_KHR,
                        .stageFlags           = VK_SHADER_STAGE_ALL,
                        .layout               = pipelineLayout->handle,
                        .set                  = firstGroup,
                        .descriptorWriteCount = 1,
                        .pDescriptorWrites    = &write,
                    };
                    vkCmdPushDescriptorSet2KHR(self->commandBuffer, &pushInfo);
                }
            }

            for (const BindGroupImagesUpdateInfo& update : updateInfo.images)
            {
                const bool storage = groupLayout->GetBinding(update.dstBinding).type == BindingType::StorageImage;
                for (size_t first = 0; first < update.images.size(); first += Limits::DescriptorBatch)
                {
                    const size_t          count = (std::min)(Limits::DescriptorBatch, update.images.size() - first);
                    VkDescriptorImageInfo infos[Limits::DescriptorBatch];
                    for (size_t i = 0; i < count; ++i)
                    {
                        const auto* image = static_cast<const IImage*>(update.images[first + i]);
                        infos[i] = {
                            .sampler     = VK_NULL_HANDLE,
                            .imageView   = image->viewHandle,
                            .imageLayout = storage ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        };
                    }
                    const VkWriteDescriptorSet write{
                        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstBinding      = update.dstBinding,
                        .dstArrayElement = update.dstArrayElement + static_cast<uint32_t>(first),
                        .descriptorCount = static_cast<uint32_t>(count),
                        .descriptorType  = storage ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                        .pImageInfo      = infos,
                    };
                    const VkPushDescriptorSetInfo pushInfo{
                        .sType                = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO_KHR,
                        .stageFlags           = VK_SHADER_STAGE_ALL,
                        .layout               = pipelineLayout->handle,
                        .set                  = firstGroup,
                        .descriptorWriteCount = 1,
                        .pDescriptorWrites    = &write,
                    };
                    vkCmdPushDescriptorSet2KHR(self->commandBuffer, &pushInfo);
                }
            }

            for (const BindGroupSamplersUpdateInfo& update : updateInfo.samplers)
            {
                for (size_t first = 0; first < update.samplers.size(); first += Limits::DescriptorBatch)
                {
                    const size_t          count = (std::min)(Limits::DescriptorBatch, update.samplers.size() - first);
                    VkDescriptorImageInfo infos[Limits::DescriptorBatch];
                    for (size_t i = 0; i < count; ++i)
                    {
                        const auto* sampler = static_cast<const ISampler*>(update.samplers[first + i]);
                        infos[i]            = {.sampler = sampler->handle};
                    }
                    const VkWriteDescriptorSet write{
                        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstBinding      = update.dstBinding,
                        .dstArrayElement = update.dstArrayElement + static_cast<uint32_t>(first),
                        .descriptorCount = static_cast<uint32_t>(count),
                        .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
                        .pImageInfo      = infos,
                    };
                    const VkPushDescriptorSetInfo pushInfo{
                        .sType                = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO_KHR,
                        .stageFlags           = VK_SHADER_STAGE_ALL,
                        .layout               = pipelineLayout->handle,
                        .set                  = firstGroup,
                        .descriptorWriteCount = 1,
                        .pDescriptorWrites    = &write,
                    };
                    vkCmdPushDescriptorSet2KHR(self->commandBuffer, &pushInfo);
                }
            }

            for (const BindGroupAccelerationStructureBindingInfo& update : updateInfo.accelerationStructures)
            {
                const VkAccelerationStructureKHR handle = static_cast<const IAccelerationStructure*>(update.accelerationStructure)->handle;

                const VkWriteDescriptorSetAccelerationStructureKHR accelerationInfo{
                    .sType                      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
                    .accelerationStructureCount = 1,
                    .pAccelerationStructures    = &handle,
                };
                const VkWriteDescriptorSet write{
                    .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext           = &accelerationInfo,
                    .dstBinding      = update.dstBinding,
                    .dstArrayElement = update.dstArrayElement,
                    .descriptorCount = 1,
                    .descriptorType  = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
                };
                const VkPushDescriptorSetInfo pushInfo{
                    .sType                = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO_KHR,
                    .stageFlags           = VK_SHADER_STAGE_ALL,
                    .layout               = pipelineLayout->handle,
                    .set                  = firstGroup,
                    .descriptorWriteCount = 1,
                    .pDescriptorWrites    = &write,
                };
                vkCmdPushDescriptorSet2KHR(self->commandBuffer, &pushInfo);
            }
        }
    }

    void cmdSetBindGroups(ICommandList* self, BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)
    {

        IPipelineLayout* pipelineLayout = (IPipelineLayout*)self->pipelineLayout;
        VkPipelineBindPoint vkBindPoint = ConvertBindPoint(bindPoint);

        TL_ASSERT(bindGroups.size() <= Limits::DescriptorSets, "Too many descriptor sets in one bind");
        size_t dynamicOffsetCount = 0;
        for (const auto& bindingInfo : bindGroups)
            dynamicOffsetCount += bindingInfo.dynamicOffsets.size();
        TL_ASSERT(dynamicOffsetCount <= Limits::DescriptorBatch, "Too many dynamic descriptor offsets in one bind");

        TL::InlineVector<VkDescriptorSet, Limits::DescriptorSets> descriptorSets;
        TL::InlineVector<uint32_t, Limits::DescriptorBatch> dynamicOffsets;

        for (const auto& bindingInfo : bindGroups)
        {
            auto bindGroup = (IBindGroup*)bindingInfo.bindGroup;
            descriptorSets.push_back(bindGroup->descriptorSet);
            for (uint32_t offset : bindingInfo.dynamicOffsets)
            {
                dynamicOffsets.push_back(offset);
            }
        }

        vkCmdBindDescriptorSets(self->commandBuffer, vkBindPoint, pipelineLayout->handle, 0, (uint32_t)descriptorSets.size(), descriptorSets.data(), (uint32_t)dynamicOffsets.size(), dynamicOffsets.data());
    }

    void cmdBindGraphicsPipeline(ICommandList* self, const GraphicsPipeline* pipelineState)
    {

        if (pipelineState == nullptr)
        {
            self->isGraphicsPipelineBound = false;
            return;
        }

        self->isGraphicsPipelineBound = true;
        IGraphicsPipeline* pipeline = (IGraphicsPipeline*)(pipelineState);
        self->pipelineLayout = pipeline->layout;
        IPipelineLayout* pipelineLayout = (IPipelineLayout*)self->pipelineLayout;
        self->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        vkCmdBindPipeline(self->commandBuffer, self->pipelineBindPoint, pipeline->handle);
    }

    void cmdBindComputePipeline(ICommandList* self, const ComputePipeline* pipelineState)
    {

        if (pipelineState == nullptr)
        {
            self->isComputePipelineBound = false;
            return;
        }

        self->isComputePipelineBound = true;
        IComputePipeline* pipeline = (IComputePipeline*)(pipelineState);
        self->pipelineLayout = pipeline->layout;
        IPipelineLayout* pipelineLayout = (IPipelineLayout*)self->pipelineLayout;
        self->pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

        vkCmdBindPipeline(self->commandBuffer, self->pipelineBindPoint, pipeline->handle);
    }

    void cmdBindRayTracingPipeline(ICommandList* self, const RayTracingPipeline* pipelineState)
    {

        if (pipelineState == nullptr)
            return;

        IRayTracingPipeline* pipeline = (IRayTracingPipeline*)(pipelineState);
        self->pipelineLayout = pipeline->layout;
        self->pipelineBindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;

        vkCmdBindPipeline(self->commandBuffer, self->pipelineBindPoint, pipeline->handle);
    }

    void cmdSetViewport(ICommandList* self, float offsetX, float offsetY, float width, float height, float minDepth, float maxDepth)
    {
        // Flip the viewport so Vulkan NDC is consitant with other APIs
        VkViewport vkViewport{
            .x = offsetX,
            .y = offsetY + height,
            .width = width,
            .height = -height,
            .minDepth = minDepth,
            .maxDepth = maxDepth,
        };
        vkCmdSetViewport(self->commandBuffer, 0, 1, &vkViewport);
        self->hasViewportSet = true;
    }

    void cmdSetScissor(ICommandList* self, int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height)
    {

        VkRect2D vkScissor{
            .offset = {offsetX, offsetY},
            .extent = {width, height},
        };
        vkCmdSetScissor(self->commandBuffer, 0, 1, &vkScissor);
        self->hasScissorSet = true;
    }

    void cmdBindVertexBuffers(ICommandList* self, uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    {

        constexpr size_t MaxVertexBuffers = 16;

        VkBuffer buffers[MaxVertexBuffers];
        VkDeviceSize offsets[MaxVertexBuffers];

        size_t vertexBufferCount = vertexBuffers.size();
        TL_ASSERT(vertexBufferCount <= MaxVertexBuffers, "Vertex buffer count exceeds MaxVertexBuffers!");

        for (size_t i = 0; i < vertexBufferCount; ++i)
        {
            const auto& bindingInfo = vertexBuffers[i];
            auto buffer = (IBuffer*)(bindingInfo.buffer);

            buffers[i] = buffer->handle;
            offsets[i] = bindingInfo.offset;
        }

        vkCmdBindVertexBuffers(self->commandBuffer, firstBinding, static_cast<uint32_t>(vertexBufferCount), buffers, offsets);
        self->hasVertexBuffer = true;
    }

    void cmdBindIndexBuffer(ICommandList* self, const BufferBindingInfo& indexBuffer, IndexType indexType)
    {

        auto buffer = (IBuffer*)(indexBuffer.buffer);
        vkCmdBindIndexBuffer(self->commandBuffer, buffer->handle, indexBuffer.offset, indexType == IndexType::uint32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
        self->hasIndexBuffer = true;
    }

    void cmdDraw(ICommandList* self, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {

        TL_ASSERT(self->isGraphicsPipelineBound && self->hasViewportSet);
        vkCmdDraw(self->commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void cmdDrawIndexed(ICommandList* self, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {

        TL_ASSERT(self->isGraphicsPipelineBound && self->hasViewportSet && self->hasScissorSet);
        vkCmdDrawIndexed(self->commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void cmdDrawMeshTasks(ICommandList* self, uint32_t x, uint32_t y, uint32_t z)
    {
        vkCmdDrawMeshTasksEXT(self->commandBuffer, x, y, z);
    }

    void cmdDrawIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {

        TL_ASSERT(self->isGraphicsPipelineBound && self->hasViewportSet && self->hasScissorSet);
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);

        if (countBuffer.buffer != nullptr)
        {
            auto countBuf = (IBuffer*)(countBuffer.buffer);
            vkCmdDrawIndirectCount(self->commandBuffer, cmdBuffer->handle, argumentBuffer.offset, countBuf->handle, countBuffer.offset, maxDrawCount, stride);
        }
        else
        {
            vkCmdDrawIndirect(self->commandBuffer, cmdBuffer->handle, argumentBuffer.offset, maxDrawCount, stride);
        }
    }

    void cmdDrawIndexedIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {

        TL_ASSERT(self->isGraphicsPipelineBound && self->hasViewportSet && self->hasScissorSet && self->hasVertexBuffer && self->hasIndexBuffer);
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);

        if (countBuffer.buffer != nullptr)
        {
            auto countBuf = (IBuffer*)(countBuffer.buffer);
            vkCmdDrawIndexedIndirectCount(self->commandBuffer, cmdBuffer->handle, argumentBuffer.offset, countBuf->handle, countBuffer.offset, maxDrawCount, stride);
        }
        else
        {
            vkCmdDrawIndexedIndirect(self->commandBuffer, cmdBuffer->handle, argumentBuffer.offset, maxDrawCount, stride);
        }
    }

    void cmdDrawMeshTasksIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)
    {
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);

        if (countBuffer.buffer != nullptr)
        {
            auto countBuf = (IBuffer*)(countBuffer.buffer);

            vkCmdDrawMeshTasksIndirectCountEXT(
                self->commandBuffer,
                cmdBuffer->handle,
                argumentBuffer.offset,
                countBuf->handle,
                countBuffer.offset,
                drawNum,
                stride);
        }
        else
        {
            vkCmdDrawMeshTasksIndirectEXT(self->commandBuffer, cmdBuffer->handle, argumentBuffer.offset, drawNum, stride);
        }
    }

    void cmdDispatch(ICommandList* self, uint32_t x, uint32_t y, uint32_t z)
    {

        TL_ASSERT(self->isComputePipelineBound);
        vkCmdDispatch(self->commandBuffer, x, y, z);
    }

    void cmdDispatchIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer)
    {

        TL_ASSERT(self->isComputePipelineBound);
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);
        vkCmdDispatchIndirect(self->commandBuffer, cmdBuffer->handle, argumentBuffer.offset);
    }

    void cmdDispatchRays(ICommandList* self, const DispatchRaysInfo& dispatchRaysDesc)
    {
        VkStridedDeviceAddressRegionKHR raygen = ConvertStridedDeviceAddressRegion(dispatchRaysDesc.raygenShader);
        VkStridedDeviceAddressRegionKHR miss = ConvertStridedDeviceAddressRegion(dispatchRaysDesc.missShaders);
        VkStridedDeviceAddressRegionKHR hit = ConvertStridedDeviceAddressRegion(dispatchRaysDesc.hitShaderGroups);
        VkStridedDeviceAddressRegionKHR callable = ConvertStridedDeviceAddressRegion(dispatchRaysDesc.callableShaders);
        vkCmdTraceRaysKHR(self->commandBuffer, &raygen, &miss, &hit, &callable, dispatchRaysDesc.x, dispatchRaysDesc.y, dispatchRaysDesc.z);
    }

    void cmdDispatchRaysIndirect(ICommandList* self, const BufferBindingInfo& argumentBuffer)
    {
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);
        VkDeviceAddress indirectDeviceAddress = cmdBuffer->address + argumentBuffer.offset;
        vkCmdTraceRaysIndirect2KHR(self->commandBuffer, indirectDeviceAddress);
    }

    void cmdCopyBuffer(ICommandList* self, const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size)
    {

        auto src = (const IBuffer*)(srcBuffer);
        auto dst = (const IBuffer*)(dstBuffer);

        VkBufferCopy bufferCopy{
            .srcOffset = srcOffset,
            .dstOffset = dstOffset,
            .size = size,
        };
        vkCmdCopyBuffer(self->commandBuffer, src->handle, dst->handle, 1, &bufferCopy);
    }

    void cmdCopyImage(ICommandList* self, const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size)
    {

        auto src = (const IImage*)(srcImage.image);
        auto dst = (const IImage*)(dstImage.image);

        VkImageCopy imageCopy{
            .srcSubresource = ConvertSubresourceLayers(srcImage, src->format),
            .srcOffset = ConvertOffset3D(srcImage.offset),
            .dstSubresource = ConvertSubresourceLayers(dstImage, dst->format),
            .dstOffset = ConvertOffset3D(dstImage.offset),
            .extent = ConvertExtent3D(size),
        };
        vkCmdCopyImage(self->commandBuffer, src->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
    }

    void cmdCopyImageToBuffer(ICommandList* self, const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer)
    {

        auto image = (const IImage*)(srcImage.image);
        auto buffer = (const IBuffer*)(dstBuffer);

        // TODO: Need to fix windows headers leaks here and use std::max/std::min

        VkExtent3D mipExtent{
            .width = max(1u, image->size.width >> srcImage.mipLevel),
            .height = max(1u, image->size.height >> srcImage.mipLevel),
            .depth = max(1u, image->size.depth >> srcImage.mipLevel),
        };
        VkBufferImageCopy bufferImageCopy{
            .bufferOffset = layout.offset,
            .bufferRowLength = 0,   // 0 = tightly packed (rows = imageExtent.width texels)
            .bufferImageHeight = 0, // 0 = tightly packed (slices = imageExtent.height rows)
            .imageSubresource = ConvertSubresourceLayers(srcImage, image->format),
            .imageOffset = ConvertOffset3D(srcImage.offset),
            .imageExtent = mipExtent,
        };
        vkCmdCopyImageToBuffer(self->commandBuffer, image->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer->handle, 1, &bufferImageCopy);
    }

    void cmdCopyBufferToImage(ICommandList* self, const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout)
    {

        auto buffer = (const IBuffer*)(srcBuffer);
        auto image = (const IImage*)(dstImage.image);

        VkExtent3D mipExtent{
            .width = max(1u, image->size.width >> dstImage.mipLevel),
            .height = max(1u, image->size.height >> dstImage.mipLevel),
            .depth = max(1u, image->size.depth >> dstImage.mipLevel),
        };
        VkBufferImageCopy bufferImageCopy{
            .bufferOffset = layout.offset,
            .bufferRowLength = 0,   // 0 = tightly packed (rows = imageExtent.width texels)
            .bufferImageHeight = 0, // 0 = tightly packed (slices = imageExtent.height rows)
            .imageSubresource = ConvertSubresourceLayers(dstImage, image->format),
            .imageOffset = ConvertOffset3D(dstImage.offset),
            .imageExtent = mipExtent,
        };
        vkCmdCopyBufferToImage(self->commandBuffer, buffer->handle, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
    }

    void cmdCopyAccelerationStructure(ICommandList* self, AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)
    {
        TL_UNREACHABLE();
    }

    void cmdCopyMicromap(ICommandList* self, Micromap* dst, const Micromap* src, CopyMode copyMode)
    {
        TL_UNREACHABLE();
    }

    void cmdBuildTlas(ICommandList* self, TL::Span<const TlasBuildInfo> buildInfos)
    {

        TL_ASSERT(buildInfos.size() <= Limits::AccelerationStructures, "Too many TLAS builds in one command");
        TL::InlineVector<VkAccelerationStructureGeometryKHR, Limits::AccelerationStructures> geometries;
        TL::InlineVector<VkAccelerationStructureBuildGeometryInfoKHR, Limits::AccelerationStructures> geometryInfos;
        TL::InlineVector<VkAccelerationStructureBuildRangeInfoKHR, Limits::AccelerationStructures> rangeInfos;
        TL::InlineVector<const VkAccelerationStructureBuildRangeInfoKHR*, Limits::AccelerationStructures> pRangeInfos;

        geometries.resize(buildInfos.size());
        geometryInfos.resize(buildInfos.size());
        rangeInfos.resize(buildInfos.size());
        pRangeInfos.resize(buildInfos.size());

        for (size_t i = 0; i < buildInfos.size(); ++i)
        {
            const auto& info = buildInfos[i];
            auto* dstAS = (IAccelerationStructure*)info.dst;
            auto* srcAS = (IAccelerationStructure*)info.src;
            auto* instanceBuf = (const IBuffer*)info.instanceBuffer;
            auto* scratchBuf = (const IBuffer*)info.scratchBuffer;

            geometries[i] = VkAccelerationStructureGeometryKHR{
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                .pNext = nullptr,
                .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
                .geometry = {
                    .instances = {
                        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                        .pNext = nullptr,
                        .arrayOfPointers = VK_FALSE,
                        .data = {.deviceAddress = instanceBuf->address + info.instanceBufferOffset},
                    },
                },
                .flags = {},
            };

            geometryInfos[i] = VkAccelerationStructureBuildGeometryInfoKHR{
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
                .pNext = nullptr,
                .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
                .flags = dstAS->buildFlags,
                .mode = srcAS ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
                .srcAccelerationStructure = srcAS ? srcAS->handle : VK_NULL_HANDLE,
                .dstAccelerationStructure = dstAS->handle,
                .geometryCount = 1,
                .pGeometries = &geometries[i],
                .ppGeometries = nullptr,
                .scratchData = {.deviceAddress = scratchBuf->address + info.scratchBufferOffset},
            };

            rangeInfos[i] = VkAccelerationStructureBuildRangeInfoKHR{
                .primitiveCount = info.instanceCount,
                .primitiveOffset = 0,
                .firstVertex = 0,
                .transformOffset = 0,
            };
            pRangeInfos[i] = &rangeInfos[i];
        }

        vkCmdBuildAccelerationStructuresKHR(self->commandBuffer, (uint32_t)geometryInfos.size(), geometryInfos.data(), pRangeInfos.data());
    }

    void cmdBuildBlas(ICommandList* self, TL::Span<const BlasBuildInfo> buildInfos)
    {

        TL_ASSERT(buildInfos.size() <= Limits::AccelerationStructures, "Too many BLAS builds in one command");
        TL::InlineVector<VkAccelerationStructureGeometryKHR, Limits::AccelerationGeometries> geometries;
        TL::InlineVector<VkAccelerationStructureBuildRangeInfoKHR, Limits::AccelerationGeometries> rangeInfos;
        TL::InlineVector<VkAccelerationStructureBuildGeometryInfoKHR, Limits::AccelerationStructures> geometryInfos;
        TL::InlineVector<const VkAccelerationStructureBuildRangeInfoKHR*, Limits::AccelerationStructures> pRangeInfos;

        uint32_t totalGeometries = 0;
        for (const auto& info : buildInfos)
            totalGeometries += (uint32_t)info.geometries.size();
        TL_ASSERT(totalGeometries <= Limits::AccelerationGeometries, "Too many BLAS geometries in one command");

        geometries.reserve(totalGeometries);
        rangeInfos.reserve(totalGeometries);
        geometryInfos.resize(buildInfos.size());
        pRangeInfos.resize(buildInfos.size());

        for (size_t i = 0; i < buildInfos.size(); ++i)
        {
            const auto& info = buildInfos[i];
            auto* dstAS = (IAccelerationStructure*)info.dst;
            auto* srcAS = (IAccelerationStructure*)info.src;
            auto* scratchBuf = (const IBuffer*)info.scratchBuffer;

            uint32_t firstGeometryIndex = (uint32_t)geometries.size();

            for (uint32_t g = 0; g < (uint32_t)info.geometries.size(); ++g)
            {
                const auto& geom = info.geometries[g];
                geometries.push_back(ConvertGeometryData(geom));

                uint32_t primitiveCount = 0;
                if (geom.geometryType == GeometryType::Triangles)
                    primitiveCount = (geom.indexCount > 0 ? geom.indexCount : geom.count) / 3;
                else
                    primitiveCount = geom.count;

                rangeInfos.push_back(VkAccelerationStructureBuildRangeInfoKHR{
                    .primitiveCount = primitiveCount,
                    .primitiveOffset = 0,
                    .firstVertex = 0,
                    .transformOffset = 0,
                });
            }

            geometryInfos[i] = VkAccelerationStructureBuildGeometryInfoKHR{
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
                .pNext = nullptr,
                .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
                .flags = dstAS->buildFlags,
                .mode = srcAS ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
                .srcAccelerationStructure = srcAS ? srcAS->handle : VK_NULL_HANDLE,
                .dstAccelerationStructure = dstAS->handle,
                .geometryCount = (uint32_t)info.geometries.size(),
                .pGeometries = geometries.data() + firstGeometryIndex,
                .ppGeometries = nullptr,
                .scratchData = {.deviceAddress = scratchBuf->address + info.scratchBufferOffset},
            };

            pRangeInfos[i] = rangeInfos.data() + firstGeometryIndex;
        }

        vkCmdBuildAccelerationStructuresKHR(self->commandBuffer, (uint32_t)geometryInfos.size(), geometryInfos.data(), pRangeInfos.data());
    }

    void cmdBuildMicromaps(ICommandList* self, TL::Span<const MicromapBuildInfo> buildInfos)
    {
        TL_UNREACHABLE();
    }

    void cmdClearBuffer(ICommandList* self, Buffer* dst, size_t offset, size_t size)
    {
        vkCmdFillBuffer(self->commandBuffer, ((IBuffer*)dst)->handle, offset, size, 0);
    }

    void cmdWriteAccelerationStructuresSizes(ICommandList* self, TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* _queryPool, uint32_t queryPoolOffset)
    {
        IQueryPool* queryPool = (IQueryPool*)_queryPool;
        TL_ASSERT(accelerationStructures.size() <= Limits::AccelerationStructures, "Too many acceleration structures in one query command");
        TL::InlineVector<VkAccelerationStructureKHR, Limits::AccelerationStructures> asHandles;
        for (const auto* as : accelerationStructures)
        {
            auto vkAS = (IAccelerationStructure*)as;
            asHandles.push_back(vkAS->handle);
        }
        vkCmdWriteAccelerationStructuresPropertiesKHR(self->commandBuffer, (uint32_t)asHandles.size(), asHandles.data(), VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, queryPool->handle, queryPoolOffset);
    }

    void cmdWriteMicromapsSizes(ICommandList* self, TL::Span<const Micromap*> micromaps, QueryPool* _queryPool, uint32_t queryPoolOffset)
    {
        IQueryPool* queryPool = (IQueryPool*)_queryPool;
        TL_ASSERT(micromaps.size() <= Limits::AccelerationStructures, "Too many micromaps in one query command");
        TL::InlineVector<VkMicromapEXT, Limits::AccelerationStructures> micromapHandles;
        for (const auto* micromap : micromaps)
        {
            auto vkMicromap = (IMicromap*)micromap;
            micromapHandles.push_back(vkMicromap->handle);
        }
        vkCmdWriteMicromapsPropertiesEXT(self->commandBuffer, (uint32_t)micromapHandles.size(), micromapHandles.data(), VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT, queryPool->handle, queryPoolOffset);
    }
} // namespace RHI::Vulkan
