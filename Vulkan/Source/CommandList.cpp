#include "CommandList.hpp"
#include "Common.hpp"
#include "Device.hpp"
#include "Resources.hpp"

#include <RHI/Format.hpp>
#include <TL/Containers/Optional.hpp>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    inline static VkDeviceAddress GetBufferDeviceAddress(IDevice* device, const BufferBindingInfo& bufferBindingInfo)
    {
        auto buffer = (IBuffer*)bufferBindingInfo.buffer;
        return buffer->address + bufferBindingInfo.offset;
    }

    inline static VkStridedDeviceAddressRegionKHR GetStridedDeviceAddressRegion(IDevice* device, const BufferBindingInfo& bufferBindingInfo, uint32_t stride, uint32_t drawCount)
    {
        auto buffer = (IBuffer*)bufferBindingInfo.buffer;
        return {
            .deviceAddress = buffer->address + bufferBindingInfo.offset,
            .stride        = stride,
            .size          = stride * drawCount,
        };
    }

    inline static VkImageSubresourceLayers ConvertSubresourceLayer(const ImageSubresourceLayers& subresource, Format format)
    {
        return VkImageSubresourceLayers{
            .aspectMask     = ConvertImageAspect(subresource.imageAspects, format),
            .mipLevel       = subresource.mipLevel,
            .baseArrayLayer = subresource.arrayBase,
            .layerCount     = subresource.arrayCount,
        };
    }

    inline static VkResolveModeFlagBits ConvertResolveMode(ResolveMode resolveMode)
    {
        switch (resolveMode)
        {
        case ResolveMode::None: return VK_RESOLVE_MODE_NONE;
        case ResolveMode::Min:  return VK_RESOLVE_MODE_MIN_BIT;
        case ResolveMode::Max:  return VK_RESOLVE_MODE_MAX_BIT;
        case ResolveMode::Avg:  return VK_RESOLVE_MODE_AVERAGE_BIT;
        }
        return {};
    }

    struct BarrierStage
    {
        VkPipelineStageFlags2 stageMask        = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2        accessMask       = VK_ACCESS_2_NONE;
        VkImageLayout         layout           = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t              queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    inline static BarrierStage ConvertBarrierState(TL_MAYBE_UNUSED const BarrierState& barrierState)
    {
        return {
            // .stageMask        = ConvertPipelineStageFlags(barrierState.stage),
            // .accessMask       = GetAccessFlags2(barrierState.usage, barrierState.access),
            // .layout           = VK_IMAGE_LAYOUT_UNDEFINED,
            // .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
    }

    inline static BarrierStage ConvertBarrierState(const ImageBarrierState& barrierState)
    {
        return {
            .stageMask        = ConvertPipelineStageFlags(barrierState.stage),
            .accessMask       = GetAccessFlags2(barrierState.usage, barrierState.access),
            .layout           = GetImageLayout(barrierState.usage, barrierState.access, ImageAspect::All),
            .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
    }

    inline static BarrierStage ConvertBarrierState(const BufferBarrierState& barrierState)
    {
        return {
            .stageMask        = ConvertPipelineStageFlags(barrierState.stage),
            .accessMask       = GetAccessFlags2(barrierState.usage, barrierState.access),
            .layout           = VK_IMAGE_LAYOUT_UNDEFINED,
            .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
    }

    ////////////////////////////////////////////////////////////////////////////////////
    /// CommandPool
    ////////////////////////////////////////////////////////////////////////////////////

    VkCommandPool CommandPool::Init(IDevice* device, QueueType queueType)
    {
        VkCommandPoolCreateInfo poolInfo = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = device->GetDeviceQueue(queueType)->GetFamilyIndex(),
        };

        VkResult result = vkCreateCommandPool(device->m_device, &poolInfo, nullptr, &m_pool);
        TL_ASSERT(result == VK_SUCCESS);
        m_allocatedCommandBuffers = 0;
        m_commandBuffers.clear();
        return m_pool;
    }

    void CommandPool::Shutdown(IDevice* device)
    {
        if (m_pool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device->m_device, m_pool, nullptr);
            m_pool = VK_NULL_HANDLE;
            m_commandBuffers.clear();
            m_allocatedCommandBuffers = 0;
        }
    }

    VkCommandBuffer CommandPool::AllocateCommandBuffer(IDevice* device)
    {
        if (m_allocatedCommandBuffers < m_commandBuffers.size())
        {
            return m_commandBuffers[m_allocatedCommandBuffers++];
        }

        VkCommandBufferAllocateInfo allocateInfo = {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = m_pool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkResult        result        = vkAllocateCommandBuffers(device->m_device, &allocateInfo, &commandBuffer);
        TL_ASSERT(result == VK_SUCCESS);
        m_commandBuffers.push_back(commandBuffer);
        return commandBuffer;
    }

    void CommandPool::Reset(IDevice* device)
    {
        vkResetCommandPool(device->m_device, m_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        m_allocatedCommandBuffers = 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    /// CommandAllocator
    ////////////////////////////////////////////////////////////////////////////////////

    ResultCode CommandAllocator::Init(IDevice* device)
    {
        m_device      = device;
        int queueType = 0;
        for (auto& p : m_queuePools)
            p.Init(m_device, (QueueType)queueType++);
        return ResultCode::Success;
    }

    void CommandAllocator::Shutdown()
    {
        for (auto& p : m_queuePools)
            p.Shutdown(m_device);
    }

    VkCommandBuffer CommandAllocator::AllocateCommandBuffer(QueueType queueType)
    {
        return m_queuePools[(uint32_t)queueType].AllocateCommandBuffer(m_device);
    }

    void CommandAllocator::Reset()
    {
        for (auto& p : m_queuePools)
            p.Reset(m_device);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    ICommandList::ICommandList()  = default;
    ICommandList::~ICommandList() = default;

    ResultCode ICommandList::Init(IDevice* device, CommandPool* pool, const CommandListCreateInfo& createInfo)
    {
        m_device        = device;
        m_commandBuffer = pool->AllocateCommandBuffer(m_device);
        return ResultCode::Success;
    }

    void ICommandList::Shutdown()
    {
        // m_device->m_commandsAllocator->ReleaseCommandBuffers(m_commandBuffer);
    }

    void ICommandList::AddPipelineBarriers(const PipelineBarriers& barriers)
    {
        ZoneScoped;

        if (barriers.memoryBarriers.empty() && barriers.bufferBarriers.empty() && barriers.imageBarriers.empty())
            return;

        VkDependencyInfo dependencyInfo =
            {
                .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .pNext                    = nullptr,
                .dependencyFlags          = 0,
                .memoryBarrierCount       = uint32_t(barriers.memoryBarriers.size()),
                .pMemoryBarriers          = barriers.memoryBarriers.data(),
                .bufferMemoryBarrierCount = uint32_t(barriers.bufferBarriers.size()),
                .pBufferMemoryBarriers    = barriers.bufferBarriers.data(),
                .imageMemoryBarrierCount  = uint32_t(barriers.imageBarriers.size()),
                .pImageMemoryBarriers     = barriers.imageBarriers.data(),
            };
        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

    void ICommandList::Begin()
    {
        ZoneScoped;

        VkCommandBufferBeginInfo beginInfo{
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext            = nullptr,
            .flags            = 0,
            .pInheritanceInfo = nullptr,
        };
        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    }

    void ICommandList::End()
    {
        ZoneScoped;

        vkEndCommandBuffer(m_commandBuffer);
    }

    void ICommandList::AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers)
    {
        ZoneScoped;

        if (barriers.empty() && imageBarriers.empty() && bufferBarriers.empty())
            return;

        TL::Vector<VkMemoryBarrier2> vmemoryBarriers{m_device->GetTempAllocator()};
        vmemoryBarriers.reserve(barriers.size());
        TL::Vector<VkBufferMemoryBarrier2> vbufferBarriers{m_device->GetTempAllocator()};
        vbufferBarriers.reserve(imageBarriers.size());
        TL::Vector<VkImageMemoryBarrier2> vimageBarriers{m_device->GetTempAllocator()};
        vimageBarriers.reserve(bufferBarriers.size());

        for (auto barrier : barriers)
        {
            auto [srcStageMask, srcAccessMask, srcLayout, srcQueueFamilyIndex] = ConvertBarrierState(barrier.srcState);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQueueFamilyIndex] = ConvertBarrierState(barrier.dstState);

            vmemoryBarriers.push_back({
                .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
                .pNext         = nullptr,
                .srcStageMask  = srcStageMask,
                .srcAccessMask = srcAccessMask,
                .dstStageMask  = dstStageMask,
                .dstAccessMask = dstAccessMask,
            });
        }

        for (auto imageBarrier : imageBarriers)
        {
            auto image = (IImage*)(imageBarrier.image);

            auto [srcStageMask, srcAccessMask, srcLayout, srcQueueFamilyIndex] = ConvertBarrierState(imageBarrier.srcState);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQueueFamilyIndex] = ConvertBarrierState(imageBarrier.dstState);

            vimageBarriers.push_back({
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext               = nullptr,
                .srcStageMask        = srcStageMask,
                .srcAccessMask       = srcAccessMask,
                .dstStageMask        = dstStageMask,
                .dstAccessMask       = dstAccessMask,
                .oldLayout           = srcLayout,
                .newLayout           = dstLayout,
                .srcQueueFamilyIndex = srcQueueFamilyIndex,
                .dstQueueFamilyIndex = dstQueueFamilyIndex,
                .image               = image->handle,
                .subresourceRange    = ConvertSubresourceRange(image->subresources, image->format),
            });
        }

        for (auto bufferBarrier : bufferBarriers)
        {
            auto buffer = (IBuffer*)(bufferBarrier.buffer);

            auto [srcStageMask, srcAccessMask, srcLayout, srcQueueFamilyIndex] = ConvertBarrierState(bufferBarrier.srcState);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQueueFamilyIndex] = ConvertBarrierState(bufferBarrier.dstState);

            vbufferBarriers.push_back({
                .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext               = nullptr,
                .srcStageMask        = srcStageMask,
                .srcAccessMask       = srcAccessMask,
                .dstStageMask        = dstStageMask,
                .dstAccessMask       = dstAccessMask,
                .srcQueueFamilyIndex = srcQueueFamilyIndex,
                .dstQueueFamilyIndex = dstQueueFamilyIndex,
                .buffer              = buffer->handle,
                .offset              = bufferBarrier.subregion.offset,
                .size                = VK_WHOLE_SIZE, // bufferBarrier.subregion.size,
            });
        }

        VkDependencyInfo dependencyInfo =
            {
                .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .pNext                    = nullptr,
                .dependencyFlags          = 0,
                .memoryBarrierCount       = uint32_t(vmemoryBarriers.size()),
                .pMemoryBarriers          = vmemoryBarriers.data(),
                .bufferMemoryBarrierCount = uint32_t(vbufferBarriers.size()),
                .pBufferMemoryBarriers    = vbufferBarriers.data(),
                .imageMemoryBarrierCount  = uint32_t(vimageBarriers.size()),
                .pImageMemoryBarriers     = vimageBarriers.data(),
            };
        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

    void ICommandList::BeginRenderPass(const RenderPassBeginInfo& beginInfo)
    {
        ZoneScoped;

        TL::Vector<VkRenderingAttachmentInfo>   colorAttachments{m_device->GetTempAllocator()};
        TL::Optional<VkRenderingAttachmentInfo> depthAttachment{};
        TL::Optional<VkRenderingAttachmentInfo> stencilAttachment{};

        for (const auto& colorAttachment : beginInfo.colorAttachments)
        {
            auto         colorImage  = (IImage*)(colorAttachment.view);
            auto         resolveView = colorAttachment.resolveView ? (IImage*)(colorAttachment.resolveView) : nullptr;
            VkClearValue clearValue  = {.color = ConvertClearValue(colorAttachment.clearValue)};
            colorAttachments.push_back({
                .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext              = nullptr,
                .imageView          = colorImage->viewHandle,
                .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode        = ConvertResolveMode(colorAttachment.resolveMode),
                .resolveImageView   = resolveView ? resolveView->viewHandle : VK_NULL_HANDLE,
                .resolveImageLayout = resolveView ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp             = ConvertLoadOp(colorAttachment.loadOp),
                .storeOp            = ConvertStoreOp(colorAttachment.storeOp),
                .clearValue         = {clearValue},
            });
        }

        if (beginInfo.depthStencilAttachment.view)
        {
            auto image      = (IImage*)(beginInfo.depthStencilAttachment.view);
            auto clearValue = ConvertDepthStencilValue(beginInfo.depthStencilAttachment.clearValue);

            if (image->subresources.imageAspects & ImageAspect::Depth)
            {
                depthAttachment = VkRenderingAttachmentInfo{
                    .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext              = nullptr,
                    .imageView          = image->viewHandle,
                    .imageLayout        = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                    .resolveMode        = VK_RESOLVE_MODE_NONE,
                    .resolveImageView   = VK_NULL_HANDLE,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .loadOp             = ConvertLoadOp(beginInfo.depthStencilAttachment.depthLoadOp),
                    .storeOp            = ConvertStoreOp(beginInfo.depthStencilAttachment.depthStoreOp),
                    .clearValue         = {.depthStencil = clearValue},
                };
            }

            if (image->subresources.imageAspects & ImageAspect::Stencil)
            {
                stencilAttachment = VkRenderingAttachmentInfo{
                    .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext              = nullptr,
                    .imageView          = image->viewHandle,
                    .imageLayout        = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                    .resolveMode        = VK_RESOLVE_MODE_NONE,
                    .resolveImageView   = VK_NULL_HANDLE,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .loadOp             = ConvertLoadOp(beginInfo.depthStencilAttachment.stencilLoadOp),
                    .storeOp            = ConvertStoreOp(beginInfo.depthStencilAttachment.stencilStoreOp),
                    .clearValue         = {.depthStencil = clearValue},
                };
            }
            if ((image->subresources.imageAspects & ImageAspect::DepthStencil) == ImageAspect::DepthStencil)
            {
                depthAttachment->imageLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                stencilAttachment->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
        }

        auto [offsetX, offsetY] = beginInfo.offset;
        auto [width, height]    = beginInfo.size;
        VkRenderingInfo renderingInfo{
            .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext                = nullptr,
            .flags                = 0,
            .renderArea           = {.offset = {offsetX, offsetY}, .extent = {width, height}},
            .layerCount           = 1,
            .viewMask             = 0,
            .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
            .pColorAttachments    = colorAttachments.data(),
            .pDepthAttachment     = depthAttachment.has_value() ? &depthAttachment.value() : nullptr,
            .pStencilAttachment   = stencilAttachment.has_value() ? &stencilAttachment.value() : nullptr,
        };
        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
    }

    void ICommandList::EndRenderPass()
    {
        ZoneScoped;
        vkCmdEndRendering(m_commandBuffer);
    }

    void ICommandList::BeginComputePass(TL_MAYBE_UNUSED const ComputePassBeginInfo& beginInfo)
    {
        // No-Op
    }

    void ICommandList::EndComputePass()
    {
        // No-Op
    }

    void ICommandList::PushDebugMarker(TL_MAYBE_UNUSED const char* name, TL_MAYBE_UNUSED uint32_t bgra)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (auto fn = m_device->m_pfn.m_vkCmdBeginDebugUtilsLabelEXT)
        {
            uint32_t             r = (bgra >> 16) & 0xFF;
            uint32_t             g = (bgra >> 8) & 0xFF;
            uint32_t             b = bgra & 0xFF;
            uint32_t             a = (bgra >> 24) & 0xFF;
            VkDebugUtilsLabelEXT info{
                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                .color      = {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f},
            };
            fn(m_commandBuffer, &info);
        }
#endif
    }

    void ICommandList::PopDebugMarker()
    {
        if (auto fn = m_device->m_pfn.m_vkCmdEndDebugUtilsLabelEXT)
        {
            fn(m_commandBuffer);
        }
    }

    void ICommandList::InsertDebugMarker(TL_MAYBE_UNUSED const char* name, TL_MAYBE_UNUSED uint32_t bgra)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (auto fn = m_device->m_pfn.m_vkCmdInsertDebugUtilsLabelEXT)
        {
            uint32_t             r = (bgra >> 16) & 0xFF;
            uint32_t             g = (bgra >> 8) & 0xFF;
            uint32_t             b = bgra & 0xFF;
            uint32_t             a = (bgra >> 24) & 0xFF;
            VkDebugUtilsLabelEXT info{
                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                .color      = {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f},
            };
            fn(m_commandBuffer, &info);
        }
#endif
    }

    void ICommandList::BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted)
    {
        ZoneScoped;

        auto buffer = (IBuffer*)(conditionBuffer.buffer);

        VkConditionalRenderingBeginInfoEXT beginInfo{
            .sType  = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT,
            .pNext  = nullptr,
            .buffer = buffer->handle,
            .offset = conditionBuffer.offset,
            .flags  = inverted ? VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT : 0u,
        };
        m_device->m_pfn.m_vkCmdBeginConditionalRenderingEXT(m_commandBuffer, &beginInfo);
    }

    void ICommandList::EndConditionalCommands()
    {
        ZoneScoped;

        m_device->m_pfn.m_vkCmdEndConditionalRenderingEXT(m_commandBuffer);
    }

    void ICommandList::Execute(TL::Span<const CommandList*> commandLists)
    {
        ZoneScoped;

        TL::Vector<VkCommandBuffer> commandBuffers{m_device->GetTempAllocator()};
        commandBuffers.reserve(commandLists.size());

        for (const auto* commandList : commandLists)
        {
            auto vkCmdList = static_cast<const ICommandList*>(commandList);
            commandBuffers.push_back(vkCmdList->m_commandBuffer);
        }

        vkCmdExecuteCommands(m_commandBuffer, commandBuffers.size(), commandBuffers.data());
    }

    void ICommandList::BindPipelineLayout(BindPoint bindPoint, const PipelineLayout* pipelineLayout)
    {
        ZoneScoped;

        m_pipelineLayout    = (PipelineLayout*)pipelineLayout;
        m_pipelineBindPoint = bindPoint == BindPoint::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    void ICommandList::SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content)
    {
        ZoneScoped;

        IPipelineLayout*    pipelineLayout = (IPipelineLayout*)m_pipelineLayout;
        VkPipelineBindPoint vkBindPoint    = bindPoint == BindPoint::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;

        vkCmdPushConstants(m_commandBuffer, pipelineLayout->handle, VK_SHADER_STAGE_ALL, offset, (uint32_t)content.size, content.ptr);
    }

    void ICommandList::PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos)
    {
        ZoneScoped;

        IPipelineLayout*    pipelineLayout = (IPipelineLayout*)m_pipelineLayout;
        IBindGroupLayout*   groupLayout    = pipelineLayout->bindGroupLayouts[firstGroup];
        DescriptorSetWriter writer{m_device, VK_NULL_HANDLE, groupLayout, m_device->GetTempAllocator()};

        VkPushDescriptorSetInfo pushDescriptorSetInfo{
            .sType                = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO_KHR,
            .pNext                = nullptr,
            .stageFlags           = VK_SHADER_STAGE_ALL,
            .layout               = pipelineLayout->handle,
            .set                  = firstGroup,
            .descriptorWriteCount = (uint32_t)writer.GetWrites().size(),
            .pDescriptorWrites    = writer.GetWrites().data(),
        };
        m_device->m_pfn.m_vkCmdPushDescriptorSet2KHR(m_commandBuffer, &pushDescriptorSetInfo);
    }

    void ICommandList::SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        IPipelineLayout*    pipelineLayout = (IPipelineLayout*)m_pipelineLayout;
        VkPipelineBindPoint vkBindPoint    = bindPoint == BindPoint::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;

        BindShaderBindGroups(vkBindPoint, pipelineLayout->handle, bindGroups);
    }

    void ICommandList::BindGraphicsPipeline(const GraphicsPipeline* pipelineState)
    {
        ZoneScoped;

        if (pipelineState == nullptr)
        {
            m_isGraphicsPipelineBound = false;
            return;
        }

        m_isGraphicsPipelineBound       = true;
        IGraphicsPipeline* pipeline     = (IGraphicsPipeline*)(pipelineState);
        m_pipelineLayout                = pipeline->layout;
        IPipelineLayout* pipelineLayout = (IPipelineLayout*)m_pipelineLayout;
        m_pipelineBindPoint             = VK_PIPELINE_BIND_POINT_GRAPHICS;

        vkCmdBindPipeline(m_commandBuffer, m_pipelineBindPoint, pipeline->handle);
    }

    void ICommandList::BindComputePipeline(const ComputePipeline* pipelineState)
    {
        ZoneScoped;

        if (pipelineState == nullptr)
        {
            m_isComputePipelineBound = false;
            return;
        }

        m_isComputePipelineBound        = true;
        IComputePipeline* pipeline      = (IComputePipeline*)(pipelineState);
        m_pipelineLayout                = pipeline->layout;
        IPipelineLayout* pipelineLayout = (IPipelineLayout*)m_pipelineLayout;
        m_pipelineBindPoint             = VK_PIPELINE_BIND_POINT_COMPUTE;

        vkCmdBindPipeline(m_commandBuffer, m_pipelineBindPoint, pipeline->handle);
    }

    void ICommandList::SetViewport(const Viewport& viewport)
    {
        ZoneScoped;
        // Flip the viewport so Vulkan NDC is consitant with other APIs
        VkViewport vkViewport{
            .x        = viewport.offsetX,
            .y        = viewport.offsetY + viewport.height,
            .width    = viewport.width,
            .height   = -viewport.height,
            .minDepth = viewport.minDepth,
            .maxDepth = viewport.maxDepth,
        };
        vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
        m_hasViewportSet = true;
    }

    void ICommandList::SetScissor(const Scissor& scissor)
    {
        ZoneScoped;

        VkRect2D vkScissor{
            .offset = {scissor.offsetX, scissor.offsetY},
            .extent = {scissor.width, scissor.height},
        };
        vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
        m_hasScissorSet = true;
    }

    void ICommandList::BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    {
        ZoneScoped;

        constexpr size_t MaxVertexBuffers = 16;

        VkBuffer     buffers[MaxVertexBuffers];
        VkDeviceSize offsets[MaxVertexBuffers];

        size_t vertexBufferCount = vertexBuffers.size();
        TL_ASSERT(vertexBufferCount <= MaxVertexBuffers, "Vertex buffer count exceeds MaxVertexBuffers!");

        for (size_t i = 0; i < vertexBufferCount; ++i)
        {
            const auto& bindingInfo = vertexBuffers[i];
            auto        buffer      = (IBuffer*)(bindingInfo.buffer);

            buffers[i] = buffer->handle;
            offsets[i] = bindingInfo.offset;
        }

        vkCmdBindVertexBuffers(m_commandBuffer, firstBinding, static_cast<uint32_t>(vertexBufferCount), buffers, offsets);
        m_hasVertexBuffer = true;
    }

    void ICommandList::BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)
    {
        ZoneScoped;

        auto buffer = (IBuffer*)(indexBuffer.buffer);
        vkCmdBindIndexBuffer(m_commandBuffer, buffer->handle, indexBuffer.offset, indexType == IndexType::uint32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
        m_hasIndexBuffer = true;
    }

    void ICommandList::Draw(const DrawParameters& parameters)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet);
        vkCmdDraw(m_commandBuffer, parameters.vertexCount, parameters.instanceCount, parameters.firstVertex, parameters.firstInstance);
    }

    void ICommandList::DrawIndexed(const DrawIndexedParameters& parameters)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet && m_hasScissorSet);
        vkCmdDrawIndexed(m_commandBuffer, parameters.indexCount, parameters.instanceCount, parameters.firstIndex, parameters.vertexOffset, parameters.firstInstance);
    }

    void ICommandList::DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet && m_hasScissorSet && m_hasVertexBuffer);
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);

        if (countBuffer.buffer != nullptr)
        {
            auto countBuf = (IBuffer*)(countBuffer.buffer);
            vkCmdDrawIndirectCount(m_commandBuffer, cmdBuffer->handle, argumentBuffer.offset, countBuf->handle, countBuffer.offset, maxDrawCount, stride);
        }
        else
        {
            vkCmdDrawIndirect(m_commandBuffer, cmdBuffer->handle, argumentBuffer.offset, maxDrawCount, stride);
        }
    }

    void ICommandList::DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet && m_hasScissorSet && m_hasVertexBuffer && m_hasIndexBuffer);
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);

        if (countBuffer.buffer != nullptr)
        {
            auto countBuf = (IBuffer*)(countBuffer.buffer);
            vkCmdDrawIndexedIndirectCount(m_commandBuffer, cmdBuffer->handle, argumentBuffer.offset, countBuf->handle, countBuffer.offset, maxDrawCount, stride);
        }
        else
        {
            vkCmdDrawIndexedIndirect(m_commandBuffer, cmdBuffer->handle, argumentBuffer.offset, maxDrawCount, stride);
        }
    }

    void ICommandList::DrawMeshTasks(const DrawMeshTasksInfo drawMeshTasksDesc)
    {
        m_device->m_pfn.m_vkCmdDrawMeshTasksEXT(m_commandBuffer, drawMeshTasksDesc.x, drawMeshTasksDesc.y, drawMeshTasksDesc.z);
    }

    void ICommandList::DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)
    {
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);

        if (countBuffer.buffer != nullptr)
        {
            auto countBuf = (IBuffer*)(countBuffer.buffer);

            m_device->m_pfn.m_vkCmdDrawMeshTasksIndirectCountEXT(
                m_commandBuffer,
                cmdBuffer->handle,
                argumentBuffer.offset,
                countBuf->handle,
                countBuffer.offset,
                drawNum,
                stride);
        }
        else
        {
            m_device->m_pfn.m_vkCmdDrawMeshTasksIndirectEXT(m_commandBuffer, cmdBuffer->handle, argumentBuffer.offset, drawNum, stride);
        }
    }

    void ICommandList::DispatchRays(const DispatchRaysInfo& dispatchRaysDesc)
    {
        VkStridedDeviceAddressRegionKHR raygenShaderBindingTable{};
        VkStridedDeviceAddressRegionKHR missShaderBindingTable{};
        VkStridedDeviceAddressRegionKHR hitShaderBindingTable{};
        VkStridedDeviceAddressRegionKHR callableShaderBindingTable{};
        m_device->m_pfn.m_vkCmdTraceRaysKHR(
            m_commandBuffer,
            &raygenShaderBindingTable,
            &missShaderBindingTable,
            &hitShaderBindingTable,
            &callableShaderBindingTable,
            dispatchRaysDesc.x,
            dispatchRaysDesc.y,
            dispatchRaysDesc.z);
    }

    void ICommandList::DispatchRaysIndirect(const BufferBindingInfo& argumentBuffer)
    {
        auto            cmdBuffer             = (IBuffer*)(argumentBuffer.buffer);
        VkDeviceAddress indirectDeviceAddress = cmdBuffer->address + argumentBuffer.offset;
        m_device->m_pfn.m_vkCmdTraceRaysIndirect2KHR(m_commandBuffer, indirectDeviceAddress);
    }

    void ICommandList::Dispatch(const DispatchParameters& parameters)
    {
        ZoneScoped;

        TL_ASSERT(m_isComputePipelineBound);
        vkCmdDispatch(
            m_commandBuffer,
            parameters.countX,
            parameters.countY,
            parameters.countZ);
    }

    void ICommandList::DispatchIndirect(const BufferBindingInfo& argumentBuffer)
    {
        ZoneScoped;

        TL_ASSERT(m_isComputePipelineBound);
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);
        vkCmdDispatchIndirect(
            m_commandBuffer,
            cmdBuffer->handle,
            argumentBuffer.offset);
    }

    void ICommandList::CopyBuffer(const BufferCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto srcBuffer = (IBuffer*)(copyInfo.srcBuffer);
        auto dstBuffer = (IBuffer*)(copyInfo.dstBuffer);

        VkBufferCopy bufferCopy{
            .srcOffset = copyInfo.srcOffset,
            .dstOffset = copyInfo.dstOffset,
            .size      = copyInfo.size,
        };
        vkCmdCopyBuffer(m_commandBuffer, srcBuffer->handle, dstBuffer->handle, 1, &bufferCopy);
    }

    void ICommandList::CopyImage(const ImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto srcImage = (IImage*)(copyInfo.srcImage);
        auto dstImage = (IImage*)(copyInfo.dstImage);

        VkImageCopy imageCopy{
            .srcSubresource = ConvertSubresourceLayer(copyInfo.srcSubresource, srcImage->format),
            .srcOffset      = ConvertOffset3D(copyInfo.srcOffset),
            .dstSubresource = ConvertSubresourceLayer(copyInfo.dstSubresource, dstImage->format),
            .dstOffset      = ConvertOffset3D(copyInfo.dstOffset),
            .extent         = ConvertExtent3D(copyInfo.srcSize),
        };
        vkCmdCopyImage(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
    }

    void ICommandList::CopyImageToBuffer(const BufferImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto buffer = (IBuffer*)(copyInfo.buffer);
        auto image  = (IImage*)(copyInfo.image);

        VkBufferImageCopy bufferImageCopy{
            .bufferOffset      = copyInfo.bufferOffset,
            .bufferRowLength   = copyInfo.bytesPerRow,
            .bufferImageHeight = copyInfo.bytesPerImage,
            .imageSubresource  = ConvertSubresourceLayer(copyInfo.subresource, image->format),
            .imageOffset       = ConvertOffset3D(copyInfo.imageOffset),
            .imageExtent       = ConvertExtent3D(copyInfo.imageSize),
        };
        vkCmdCopyImageToBuffer(m_commandBuffer, image->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer->handle, 1, &bufferImageCopy);
    }

    void ICommandList::CopyBufferToImage(const BufferImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto buffer = (IBuffer*)(copyInfo.buffer);
        auto image  = (IImage*)(copyInfo.image);

        VkBufferImageCopy bufferImageCopy{
            .bufferOffset      = copyInfo.bufferOffset,
            .bufferRowLength   = copyInfo.bytesPerRow,
            .bufferImageHeight = copyInfo.bytesPerImage,
            .imageSubresource  = ConvertSubresourceLayer(copyInfo.subresource, image->format),
            .imageOffset       = ConvertOffset3D(copyInfo.imageOffset),
            .imageExtent       = ConvertExtent3D(copyInfo.imageSize),
        };
        vkCmdCopyBufferToImage(m_commandBuffer, buffer->handle, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
    }

    void ICommandList::BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        if (bindGroups.empty())
            return;

        TL::Vector<VkDescriptorSet> descriptorSets{m_device->GetTempAllocator()};
        TL::Vector<uint32_t>        dynamicOffsets{m_device->GetTempAllocator()};

        for (const auto& bindingInfo : bindGroups)
        {
            auto bindGroup = (IBindGroup*)bindingInfo.bindGroup;
            descriptorSets.push_back(bindGroup->descriptorSet);
            for (uint32_t offset : bindingInfo.dynamicOffsets)
            {
                dynamicOffsets.push_back(offset);
            }
        }

#if 1 // todo: use the vkCmdBindDescriptorSets2
        vkCmdBindDescriptorSets(m_commandBuffer, bindPoint, pipelineLayout, 0, (uint32_t)descriptorSets.size(), descriptorSets.data(), dynamicOffsets.size(), dynamicOffsets.data());
#else
        VkBindDescriptorSetsInfo bindInfo{
            .sType              = VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO,
            .pNext              = nullptr,
            .stageFlags         = VK_SHADER_STAGE_ALL,
            .layout             = pipelineLayout,
            .firstSet           = 0,
            .descriptorSetCount = (uint32_t)descriptorSets.size(),
            .pDescriptorSets    = descriptorSets.data(),
            .dynamicOffsetCount = (uint32_t)dynamicOffsets.size(),
            .pDynamicOffsets    = dynamicOffsets.data(),
        };
        vkCmdBindDescriptorSets2(m_commandBuffer, &bindInfo);
#endif
    }

    void ICommandList::BuildMicromaps(TL::Span<const BuildMicromapDesc> buildMicromapDescs)
    {
    }

    void ICommandList::WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
    }

    void ICommandList::CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode)
    {
    }

    void ICommandList::BuildTopLevelAccelerationStructures(TL::Span<const BuildTopLevelAccelerationStructureDesc> buildTopLevelAccelerationStructureDescs)
    {
    }

    void ICommandList::BuildBottomLevelAccelerationStructures(TL::Span<const BuildBottomLevelAccelerationStructureDesc> buildBottomLevelAccelerationStructureDescs)
    {
    }

    void ICommandList::WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* queryPool, uint32_t queryPoolOffset)
    {
    }

    void ICommandList::CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)
    {
    }

} // namespace RHI::Vulkan
