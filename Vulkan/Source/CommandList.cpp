#include "CommandList.hpp"

#include <RHI/Format.hpp>

#include <tracy/Tracy.hpp>

#include "Common.hpp"
#include "Device.hpp"
#include "Resources.hpp"
#include "Swapchain.hpp"

namespace RHI::Vulkan
{
    inline static VkImageSubresourceLayers ConvertSubresourceLayer(const ImageSubresourceLayers& subresource, Format format)
    {
        auto vkSubresource           = VkImageSubresourceLayers{};
        vkSubresource.aspectMask     = ConvertImageAspect(subresource.imageAspects, format);
        vkSubresource.mipLevel       = subresource.mipLevel;
        vkSubresource.baseArrayLayer = subresource.arrayBase;
        vkSubresource.layerCount     = subresource.arrayCount;
        return vkSubresource;
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

    inline BarrierStage ConvertBarrierState(const BarrierState& barrierState)
    {
        return {
            // .stageMask        = ConvertPipelineStageFlags(barrierState.stage),
            // .accessMask       = GetAccessFlags2(barrierState.usage, barrierState.access),
            // .layout           = VK_IMAGE_LAYOUT_UNDEFINED,
            // .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
    }

    inline BarrierStage ConvertBarrierState(const ImageBarrierState& barrierState)
    {
        return {
            .stageMask        = ConvertPipelineStageFlags(barrierState.stage),
            .accessMask       = GetAccessFlags2(barrierState.usage, barrierState.access),
            .layout           = GetImageLayout(barrierState.usage, barrierState.access, ImageAspect::All),
            .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
    }

    inline BarrierStage ConvertBarrierState(const BufferBarrierState& barrierState)
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

    CommandPool::CommandPool()  = default;
    CommandPool::~CommandPool() = default;

    ResultCode CommandPool::Init(IDevice* device)
    {
        m_device = device;

        // Lock mutex while initializing pools
        std::lock_guard<std::mutex> lock(m_poolMutex);

        // Initialize command pools for each queue type per thread
        auto& poolMap = m_pools[std::this_thread::get_id()];

        for (size_t i = 0; i < (int)QueueType::Count; ++i)
        {
            poolMap[i] = CreateCommandPool(static_cast<QueueType>(i));
        }

        return ResultCode::Success;
    }

    void CommandPool::Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        DestroyCommandPools();
    }

    VkCommandBuffer CommandPool::AllocateCommandBuffer(QueueType queueType)
    {
        // Lock mutex for thread-safe pool access
        std::lock_guard<std::mutex> lock(m_poolMutex);

        // Get command pool for this queue type and create a command buffer
        VkCommandPool pool = m_pools[std::this_thread::get_id()][(uint32_t)queueType];

        VkCommandBufferAllocateInfo allocateInfo = {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = pool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkResult        result        = vkAllocateCommandBuffers(m_device->m_device, &allocateInfo, &commandBuffer);

        if (result != VK_SUCCESS)
        {
            TL_UNREACHABLE_MSG("Failed to allocate Vulkan command buffer");
            return nullptr;
        }

        return commandBuffer;
    }

    void CommandPool::ReleaseCommandBuffers([[maybe_unused]] TL::Span<const VkCommandBuffer> commandBuffers)
    {
        // Commands are all transient nothing to do here.
    }

    void CommandPool::Reset()
    {
        // Lock mutex for thread-safe reset
        std::lock_guard<std::mutex> lock(m_poolMutex);

        // Reset command pools for the current thread
        auto& poolMap = m_pools[std::this_thread::get_id()];

        for (size_t i = 0; i < (int)QueueType::Count; ++i)
        {
            VkCommandPool pool = poolMap[i];
            vkResetCommandPool(m_device->m_device, pool, 0);
        }
    }

    VkCommandPool CommandPool::CreateCommandPool(QueueType queueType)
    {
        VkCommandPoolCreateInfo poolCreateInfo = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_device->m_queue[(uint32_t)queueType].GetFamilyIndex(),
        };

        VkCommandPool pool;
        VkResult      result = vkCreateCommandPool(m_device->m_device, &poolCreateInfo, nullptr, &pool);

        if (result != VK_SUCCESS)
        {
            // Handle error
            TL_UNREACHABLE_MSG("Failed to create Vulkan command pool");
        }

        return pool;
    }

    void CommandPool::DestroyCommandPools()
    {
        // Destroy command pools for the current thread
        auto& poolMap = m_pools[std::this_thread::get_id()];

        for (size_t i = 0; i < (int)QueueType::Count; ++i)
        {
            VkCommandPool pool = poolMap[i];
            vkDestroyCommandPool(m_device->m_device, pool, nullptr);
        }

        m_pools.erase(std::this_thread::get_id()); // Clean up the pools for this thread
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    ICommandList::ICommandList()  = default;
    ICommandList::~ICommandList() = default;

    ResultCode ICommandList::Init(IDevice* device, const CommandListCreateInfo& createInfo)
    {
        m_device        = device;
        m_commandBuffer = m_device->m_commandsAllocator->AllocateCommandBuffer(createInfo.queueType);
        return ResultCode::Success;
    }

    void ICommandList::Shutdown()
    {
        m_device->m_commandsAllocator->ReleaseCommandBuffers(m_commandBuffer);
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

        TL::Vector<VkMemoryBarrier2>       vmemoryBarriers;
        TL::Vector<VkBufferMemoryBarrier2> vbufferBarriers;
        TL::Vector<VkImageMemoryBarrier2>  vimageBarriers;

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
                .dstAccessMask = srcAccessMask,
            });
        }

        for (auto imageBarrier : imageBarriers)
        {
            auto image = m_device->m_imageOwner.Get(imageBarrier.image);

            auto [srcStageMask, srcAccessMask, srcLayout, srcQueueFamilyIndex] = ConvertBarrierState(imageBarrier.srcState);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQueueFamilyIndex] = ConvertBarrierState(imageBarrier.dstState);

            vimageBarriers.push_back({
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext               = nullptr,
                .srcStageMask        = srcStageMask,
                .srcAccessMask       = srcAccessMask,
                .dstStageMask        = dstStageMask,
                .dstAccessMask       = srcAccessMask,
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
            auto buffer = m_device->m_bufferOwner.Get(bufferBarrier.buffer);

            auto [srcStageMask, srcAccessMask, srcLayout, srcQueueFamilyIndex] = ConvertBarrierState(bufferBarrier.srcState);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQueueFamilyIndex] = ConvertBarrierState(bufferBarrier.dstState);

            vbufferBarriers.push_back({
                .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext               = nullptr,
                .srcStageMask        = srcStageMask,
                .srcAccessMask       = srcAccessMask,
                .dstStageMask        = dstStageMask,
                .dstAccessMask       = srcAccessMask,
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

        TL::Vector<VkRenderingAttachmentInfo>   colorAttachments  = {};
        TL::Optional<VkRenderingAttachmentInfo> depthAttachment   = {};
        TL::Optional<VkRenderingAttachmentInfo> stencilAttachment = {};

        for (const auto& colorAttachment : beginInfo.colorAttachments)
        {
            auto         colorImage  = m_device->m_imageOwner.Get(colorAttachment.view);
            auto         resolveView = colorAttachment.resolveView ? m_device->m_imageOwner.Get(colorAttachment.resolveView) : nullptr;
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

        if (auto depthStencilAttachment = beginInfo.depthStencilAttachment)
        {
            auto image      = m_device->m_imageOwner.Get(depthStencilAttachment->view);
            auto clearValue = ConvertDepthStencilValue(depthStencilAttachment->clearValue);

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
                    .loadOp             = ConvertLoadOp(depthStencilAttachment->depthLoadOp),
                    .storeOp            = ConvertStoreOp(depthStencilAttachment->depthStoreOp),
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
                    .loadOp             = ConvertLoadOp(depthStencilAttachment->stencilLoadOp),
                    .storeOp            = ConvertStoreOp(depthStencilAttachment->stencilStoreOp),
                    .clearValue         = {.depthStencil = clearValue},
                };
            }
            if ((image->subresources.imageAspects & ImageAspect::DepthStencil) == ImageAspect::DepthStencil)
            {
                depthAttachment->imageLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                stencilAttachment->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
        }

        VkRenderingInfo renderingInfo{
            .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext                = nullptr,
            .flags                = 0,
            .renderArea           = {.offset = ConvertOffset2D(beginInfo.offset), .extent = ConvertExtent2D(beginInfo.size)},
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

    void ICommandList::BeginComputePass(const ComputePassBeginInfo& beginInfo)
    {
        // No-Op
    }

    void ICommandList::EndComputePass()
    {
        // No-Op
    }

    void ICommandList::PushDebugMarker(const char* name, ClearValue color)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (auto fn = m_device->m_pfn.m_vkCmdBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT info{
                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                // .color      = {(float)color.r, (float)color.g, (float)color.b, (float)color.a},
            };
            fn(m_commandBuffer, &info);
        }
#else
        (void)name;
        (void)color;
#endif
    }

    void ICommandList::PopDebugMarker()
    {
        ZoneScoped;

#if RHI_DEBUG
        if (auto fn = m_device->m_pfn.m_vkCmdEndDebugUtilsLabelEXT)
        {
            fn(m_commandBuffer);
        }
#endif
    }

    void ICommandList::BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted)
    {
        ZoneScoped;

        auto buffer = m_device->m_bufferOwner.Get(conditionBuffer.buffer);

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

        constexpr size_t MaxCommandLists = 16;
        VkCommandBuffer  commandBuffers[MaxCommandLists];
        size_t           commandListCount = commandLists.size();
        TL_ASSERT(commandListCount <= MaxCommandLists, "Exceeded MaxCommandLists limit!");
        for (size_t i = 0; i < commandListCount; ++i)
        {
            auto commandList  = (const ICommandList*)commandLists[i];
            commandBuffers[i] = commandList->m_commandBuffer;
        }
        vkCmdExecuteCommands(m_commandBuffer, static_cast<uint32_t>(commandListCount), commandBuffers);
    }

    void ICommandList::BindGraphicsPipeline(Handle<GraphicsPipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        if (pipelineState == NullHandle)
        {
            m_isGraphicsPipelineBound = false;
            return;
        }

        auto pipeline = m_device->m_graphicsPipelineOwner.Get(pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
        BindShaderBindGroups(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, bindGroups);
        m_isGraphicsPipelineBound = true;
    }

    void ICommandList::BindComputePipeline(Handle<ComputePipeline> pipelineState, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        if (pipelineState == NullHandle)
        {
            m_isComputePipelineBound = false;
            return;
        }

        auto pipeline = m_device->m_computePipelineOwner.Get(pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);
        BindShaderBindGroups(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->layout, bindGroups);
        m_isComputePipelineBound = true;
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
            .extent = {scissor.width,   scissor.height },
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
            auto        buffer      = m_device->m_bufferOwner.Get(bindingInfo.buffer);

            buffers[i] = buffer->handle;
            offsets[i] = bindingInfo.offset;
        }

        vkCmdBindVertexBuffers(
            m_commandBuffer,
            firstBinding,
            static_cast<uint32_t>(vertexBufferCount),
            buffers,
            offsets);
        m_hasVertexBuffer = true;
    }

    void ICommandList::BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)
    {
        ZoneScoped;

        auto buffer = m_device->m_bufferOwner.Get(indexBuffer.buffer);
        vkCmdBindIndexBuffer(
            m_commandBuffer,
            buffer->handle,
            indexBuffer.offset,
            indexType == IndexType::uint32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
        m_hasIndexBuffer = true;
    }

    void ICommandList::Draw(const DrawParameters& parameters)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet && m_hasScissorSet && m_hasVertexBuffer);
        vkCmdDraw(
            m_commandBuffer,
            parameters.vertexCount,
            parameters.instanceCount,
            parameters.firstVertex,
            parameters.firstInstance);
    }

    void ICommandList::DrawIndexed(const DrawIndexedParameters& parameters)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet && m_hasScissorSet && m_hasVertexBuffer && m_hasIndexBuffer);
        vkCmdDrawIndexed(
            m_commandBuffer,
            parameters.indexCount,
            parameters.instanceCount,
            parameters.firstIndex,
            parameters.vertexOffset,
            parameters.firstInstance);
    }

    void ICommandList::DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet && m_hasScissorSet && m_hasVertexBuffer);
        auto cmdBuffer = m_device->m_bufferOwner.Get(argumentBuffer.buffer);

        if (countBuffer.buffer != NullHandle)
        {
            auto countBuf = m_device->m_bufferOwner.Get(countBuffer.buffer);
            vkCmdDrawIndirectCount(
                m_commandBuffer,
                cmdBuffer->handle,
                argumentBuffer.offset,
                countBuf->handle,
                countBuffer.offset,
                maxDrawCount,
                stride);
        }
        else
        {
            vkCmdDrawIndirect(
                m_commandBuffer,
                cmdBuffer->handle,
                argumentBuffer.offset,
                maxDrawCount,
                stride);
        }
    }

    void ICommandList::DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet && m_hasScissorSet && m_hasVertexBuffer && m_hasIndexBuffer);
        auto cmdBuffer = m_device->m_bufferOwner.Get(argumentBuffer.buffer);

        if (countBuffer.buffer != NullHandle)
        {
            auto countBuf = m_device->m_bufferOwner.Get(countBuffer.buffer);
            vkCmdDrawIndexedIndirectCount(
                m_commandBuffer,
                cmdBuffer->handle,
                argumentBuffer.offset,
                countBuf->handle,
                countBuffer.offset,
                maxDrawCount,
                stride);
        }
        else
        {
            vkCmdDrawIndexedIndirect(
                m_commandBuffer,
                cmdBuffer->handle,
                argumentBuffer.offset,
                maxDrawCount,
                stride);
        }
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
        auto cmdBuffer = m_device->m_bufferOwner.Get(argumentBuffer.buffer);
        vkCmdDispatchIndirect(
            m_commandBuffer,
            cmdBuffer->handle,
            argumentBuffer.offset);
    }

    void ICommandList::CopyBuffer(const BufferCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto srcBuffer = m_device->m_bufferOwner.Get(copyInfo.srcBuffer);
        auto dstBuffer = m_device->m_bufferOwner.Get(copyInfo.dstBuffer);

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

        auto srcImage = m_device->m_imageOwner.Get(copyInfo.srcImage);
        auto dstImage = m_device->m_imageOwner.Get(copyInfo.dstImage);

        VkImageCopy imageCopy{
            .srcSubresource = ConvertSubresourceLayer(copyInfo.srcSubresource, srcImage->format),
            .srcOffset      = ConvertOffset3D(copyInfo.srcOffset),
            .dstSubresource = ConvertSubresourceLayer(copyInfo.dstSubresource, dstImage->format),
            .dstOffset      = ConvertOffset3D(copyInfo.dstOffset),
            .extent         = ConvertExtent3D(copyInfo.srcSize),
        };
        vkCmdCopyImage(
            m_commandBuffer,
            srcImage->handle,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImage->handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopy);
    }

    void ICommandList::CopyImageToBuffer(const BufferImageCopyInfo& copyInfo)
    {
        ZoneScoped;

        auto buffer = m_device->m_bufferOwner.Get(copyInfo.buffer);
        auto image  = m_device->m_imageOwner.Get(copyInfo.image);

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

        auto buffer = m_device->m_bufferOwner.Get(copyInfo.buffer);
        auto image  = m_device->m_imageOwner.Get(copyInfo.image);

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

        if (bindGroups.empty()) return;

        constexpr size_t MaxDescriptorSets = 4;
        constexpr size_t MaxDynamicOffsets = MaxDescriptorSets;

        VkDescriptorSet descriptorSets[MaxDescriptorSets];
        uint32_t        dynamicOffsets[MaxDynamicOffsets];

        uint32_t descriptorSetCount = 0;
        uint32_t dynamicOffsetCount = 0;

        for (const auto& bindingInfo : bindGroups)
        {
            auto bindGroup = m_device->m_bindGroupOwner.Get(bindingInfo.bindGroup);

            // Ensure we don't exceed the array limits
            TL_ASSERT(descriptorSetCount < MaxDescriptorSets);
            descriptorSets[descriptorSetCount++] = bindGroup->descriptorSet;

            for (uint32_t offset : bindingInfo.dynamicOffsets)
            {
                TL_ASSERT(dynamicOffsetCount < MaxDynamicOffsets);
                dynamicOffsets[dynamicOffsetCount++] = offset;
            }
        }

        vkCmdBindDescriptorSets(
            m_commandBuffer,
            bindPoint,
            pipelineLayout,
            0,
            descriptorSetCount,
            descriptorSets,
            dynamicOffsetCount,
            dynamicOffsets);
    }

} // namespace RHI::Vulkan
