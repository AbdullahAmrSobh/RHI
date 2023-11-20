#include "CommandList.hpp"

#include "Common.hpp"
#include "Context.hpp"
#include "FrameGraph.hpp"
#include "Resources.hpp"

#include <RHI/Format.hpp>

namespace Vulkan
{

    struct PipelineBarrierFlags
    {
        VkPipelineStageFlags2 stages;
        VkAccessFlags2        access;
        VkImageLayout         attachmentLayout;
    };

    inline static bool IsWriteAccess(RHI::AttachmentAccess access)
    {
        return access == RHI::AttachmentAccess::Write || access == RHI::AttachmentAccess::ReadWrite;
    }

    inline static VkPipelineStageFlags2 GetPipelineShaderStages(RHI::Flags<RHI::PipelineAccessStage> stages)
    {
        auto _stages = VkPipelineStageFlags2{};

        if (stages & RHI::PipelineAccessStage::Vertex)
        {
            _stages = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        }

        if (stages & RHI::PipelineAccessStage::Pixel)
        {
            _stages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        }

        if (stages & RHI::PipelineAccessStage::Compute)
        {
            _stages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        }

        return _stages;
    }

    inline static PipelineBarrierFlags ConvertPipelineStageAccessFlags(RHI::AttachmentUsage usage, RHI::Flags<RHI::PipelineAccessStage> stages, RHI::AttachmentAccess access)
    {
        VkPipelineStageFlags2 _stages;
        VkAccessFlags2        _access;
        VkImageLayout         _layout;

        switch (usage)
        {
        case RHI::AttachmentUsage::None:
            {
                RHI_UNREACHABLE();
                break;
            }
        case RHI::AttachmentUsage::VertexInputBuffer:
            {
                _stages = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
                _access = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
                _layout = VK_IMAGE_LAYOUT_MAX_ENUM;
                break;
            }
        case RHI::AttachmentUsage::ShaderStorage:
            {
                _stages = GetPipelineShaderStages(stages);
                RHI_ASSERT(stages);

                switch (access)
                {
                case RHI::AttachmentAccess::Read:
                    {
                        _access = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
                        _layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        break;
                    }
                case RHI::AttachmentAccess::Write:
                    {
                        _access = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;

                        break;
                    }
                case RHI::AttachmentAccess::ReadWrite:
                    {
                        _access = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
                        break;
                    }
                default:
                    {
                        RHI_UNREACHABLE();
                        break;
                    }
                }

                break;
            }
        case RHI::AttachmentUsage::RenderTarget:
            {
                _stages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                _access = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                _layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                break;
            }
        case RHI::AttachmentUsage::Depth:
            {
                _stages = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
                _access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                _layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                break;
            }
        case RHI::AttachmentUsage::ShaderResource:
            {
                _stages = GetPipelineShaderStages(stages);

                if (_stages & VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT)
                    _access |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;

                if (_stages & VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT)
                    _access |= IsWriteAccess(access) ? VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT : VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;

                if (_stages == VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT)
                    _access |= IsWriteAccess(access) ? VK_ACCESS_2_MEMORY_WRITE_BIT : VK_ACCESS_2_MEMORY_READ_BIT;

                _layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                break;
            }
        case RHI::AttachmentUsage::Copy:
            {
                _stages = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                _access = IsWriteAccess(access) ? VK_ACCESS_2_TRANSFER_WRITE_BIT : VK_ACCESS_2_TRANSFER_READ_BIT;
                _layout = IsWriteAccess(access) ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                break;
            }
        case RHI::AttachmentUsage::Resolve:
            {
                _stages = VK_PIPELINE_STAGE_2_RESOLVE_BIT;
                _access = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                _layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                break;
            }
        default:
            {
                RHI_UNREACHABLE();
                break;
            };
        }

        return { _stages, _access, _layout };
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandPool
    //////////////////////////////////////////////////////////////////////////////////////////

    VkResult CommandPool::Init(Context* context, uint32_t queueFamilyIndex)
    {
        auto createInfo             = VkCommandPoolCreateInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        return vkCreateCommandPool(context->m_device, &createInfo, nullptr, &m_commandPool);
    }

    void CommandPool::Shutdown(Context* context)
    {
        vkDestroyCommandPool(context->m_device, m_commandPool, nullptr);
    }

    void CommandPool::Reset(Context* context)
    {
        vkResetCommandPool(context->m_device, m_commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    }

    CommandList* CommandPool::Allocate(Context* context)
    {
        if (m_availableCommandLists.empty())
        {
            auto allocateInfo               = VkCommandBufferAllocateInfo{};
            allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocateInfo.pNext              = nullptr;
            allocateInfo.commandPool        = m_commandPool;
            allocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocateInfo.commandBufferCount = 1;

            auto commandBuffer = VkCommandBuffer{ VK_NULL_HANDLE };
            auto result        = vkAllocateCommandBuffers(context->m_device, &allocateInfo, &commandBuffer);
            VULKAN_ASSERT_SUCCESS(result);

            auto commandList = m_commandLists.emplace_back(std::make_unique<CommandList>(context, commandBuffer)).get();
            m_availableCommandLists.push_back(commandList);
        }

        auto commandList = m_availableCommandLists.back();
        m_availableCommandLists.pop_back();
        return commandList;
    }

    void CommandPool::Release(Context* context, CommandList* commandList)
    {
        m_availableCommandLists.push_back(commandList);
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    /// CommandListAllocator
    ////////////////////////////////////////////////////////////////////////////////////////

    CommandListAllocator::~CommandListAllocator()
    {
        for (auto& pool : m_commandPools)
        {
            pool.Shutdown(m_context);
        }
    }

    VkResult CommandListAllocator::Init(uint32_t queueFamilyIndex, uint32_t frameCount)
    {
        for (uint32_t i = 0; i < frameCount; i++)
        {
            auto& pool   = m_commandPools.emplace_back();
            auto  result = pool.Init(m_context, queueFamilyIndex);
            VULKAN_ASSERT_SUCCESS(result);
        }

        return VK_SUCCESS;
    }

    void CommandListAllocator::SetFrameIndex(uint32_t frameIndex)
    {
        auto& pool = m_commandPools[frameIndex];
        pool.Reset(m_context);
        m_frameIndex = frameIndex;
    }

    CommandList* CommandListAllocator::Allocate()
    {
        auto& pool = m_commandPools[m_frameIndex];
        return pool.Allocate(m_context);
    }

    void CommandListAllocator::Release(CommandList* commandList)
    {
        auto& pool = m_commandPools[m_frameIndex];
        pool.Release(m_context, commandList);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    void CommandList::Begin()
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext            = nullptr;
        beginInfo.flags            = 0;
        beginInfo.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    }

    void CommandList::End()
    {
        vkEndCommandBuffer(m_commandBuffer);
    }

    void CommandList::Reset()
    {
        vkResetCommandBuffer(m_commandBuffer, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    }

    void CommandList::RenderingBegin(Pass& pass)
    {
        std::vector<RHI::ImagePassAttachment*> passAttachments;

        for (auto& attachment : pass.m_imagePassAttachments)
        {
            passAttachments.push_back(&attachment);
        }
        TransitionPassAttachments(BarrierType::PrePass, passAttachments);

        std::vector<VkRenderingAttachmentInfo>   colorAttachmentInfo;
        std::optional<VkRenderingAttachmentInfo> depthAttachmentInfo;

        for (const auto& passAttachment : pass.m_imagePassAttachments)
        {
            if (passAttachment.info.usage == RHI::AttachmentUsage::Depth)
            {
                depthAttachmentInfo = GetAttachmentInfo(passAttachment);
                continue;
            }

            auto attachmentInfo = GetAttachmentInfo(passAttachment);
            colorAttachmentInfo.push_back(attachmentInfo);
        }

        auto renderingInfo                     = VkRenderingInfo{};
        renderingInfo.sType                    = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext                    = nullptr;
        renderingInfo.flags                    = 0;
        renderingInfo.renderArea.extent.width  = 800;
        renderingInfo.renderArea.extent.height = 600;
        renderingInfo.renderArea.offset.x      = 0;
        renderingInfo.renderArea.offset.y      = 0;
        renderingInfo.layerCount               = 1;
        renderingInfo.colorAttachmentCount     = colorAttachmentInfo.size();
        renderingInfo.pColorAttachments        = colorAttachmentInfo.data();
        renderingInfo.pDepthAttachment         = depthAttachmentInfo.has_value() ? &depthAttachmentInfo.value() : nullptr;
        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
    }

    void CommandList::RenderingEnd(Pass& pass)
    {
        vkCmdEndRendering(m_commandBuffer);
        std::vector<RHI::ImagePassAttachment*> passAttachments;
        for (auto& attachment : pass.m_imagePassAttachments)
        {
            passAttachments.push_back(&attachment);
        }
        TransitionPassAttachments(BarrierType::PostPass, passAttachments);
    }

    void CommandList::PushDebugMarker(const char* name)
    {
#if RHI_DEBUG
        if (m_context->m_vkCmdDebugMarkerBeginEXT)
        {
            VkDebugMarkerMarkerInfoEXT info{};
            info.sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            info.pNext       = nullptr;
            info.pMarkerName = name;
            // info.color
            m_context->m_vkCmdDebugMarkerBeginEXT(m_commandBuffer, &info);
        }
#endif
    }

    void CommandList::PopDebugMarker()
    {
#if RHI_DEUG
        if (m_context->m_vkCmdDebugMarkerEndEXT)
        {
            m_context->m_vkCmdDebugMarkerEndEXT(m_commandBuffer);
        }
#endif
    }

    void CommandList::SetViewport(const RHI::Viewport& viewport)
    {
        VkViewport vkViewport{};
        vkViewport.x        = viewport.offsetX;
        vkViewport.y        = viewport.offsetY;
        vkViewport.width    = viewport.width;
        vkViewport.height   = viewport.height;
        vkViewport.minDepth = viewport.minDepth;
        vkViewport.maxDepth = viewport.maxDepth;

        vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
    }

    void CommandList::SetSicssor(const RHI::Scissor& scissor)
    {
        VkRect2D vkScissor{};
        vkScissor.extent.width  = scissor.width;
        vkScissor.extent.height = scissor.height;
        vkScissor.offset.x      = scissor.offsetX;
        vkScissor.offset.y      = scissor.offsetY;
        vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
    }

    void CommandList::Submit(const RHI::CommandDraw& command)
    {
        auto pipeline = m_context->m_graphicsPipelineOwner.Get(command.pipelineState);

        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

        if (command.BindGroups.size())
        {
            std::vector<VkDescriptorSet> descriptorSets;
            std::vector<uint32_t>        descriptorSetOffsets;

            for (auto bindGroup : command.BindGroups)
            {
                auto descriptorSet = m_context->m_bindGroupOwner.Get(bindGroup);
                descriptorSets.push_back(descriptorSet->handle);
                descriptorSetOffsets.push_back(0);
            }

            vkCmdBindDescriptorSets(
                m_commandBuffer,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                pipeline->layout,
                0,
                static_cast<uint32_t>(descriptorSets.size()),
                descriptorSets.data(),
                static_cast<uint32_t>(descriptorSetOffsets.size()),
                descriptorSetOffsets.data());
        }

        std::vector<VkBuffer>     vertexBuffers;
        std::vector<VkDeviceSize> vertexBufferSizes;

        for (auto vertexBuffer : command.vertexBuffers)
        {
            auto buffer = m_context->m_bufferOwner.Get(vertexBuffer);

            vertexBuffers.push_back(buffer->handle);
            vertexBufferSizes.push_back(0);
        }

        if (command.vertexBuffers.size())
        {
            vkCmdBindVertexBuffers(
                m_commandBuffer,
                0,
                static_cast<uint32_t>(vertexBuffers.size()),
                vertexBuffers.data(),
                vertexBufferSizes.data());
        }

        if (command.indexBuffers)
        {
            auto buffer = m_context->m_bufferOwner.Get(command.indexBuffers);

            vkCmdBindIndexBuffer(m_commandBuffer, buffer->handle, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(
                m_commandBuffer,
                command.parameters.elementCount,
                command.parameters.instanceCount,
                command.parameters.firstElement,
                command.parameters.vertexOffset,
                static_cast<int32_t>(command.parameters.firstInstance));
        }
        else
        {
            vkCmdDraw(
                m_commandBuffer,
                command.parameters.elementCount,
                command.parameters.instanceCount,
                command.parameters.firstElement,
                command.parameters.firstInstance);
        }
    }

    void CommandList::Submit(const RHI::CommandCopy& _command)
    {
        switch (_command.type)
        {
        case RHI::CopyCommandType::Buffer:
            {
                auto& command = _command.buffer;

                auto srcBuffer         = m_context->m_bufferOwner.Get(command.sourceBuffer);
                auto destinationBuffer = m_context->m_bufferOwner.Get(command.destinationBuffer);

                auto copyInfo      = VkBufferCopy{};
                copyInfo.srcOffset = command.sourceOffset;
                copyInfo.dstOffset = command.destinationOffset;
                copyInfo.size      = command.size;
                vkCmdCopyBuffer(m_commandBuffer, srcBuffer->handle, destinationBuffer->handle, 1, &copyInfo);
                break;
            }
        case RHI::CopyCommandType::Image:
            {
                auto& command = _command.image;

                auto srcImage = m_context->m_imageOwner.Get(command.sourceImage);
                auto dstImage = m_context->m_imageOwner.Get(command.destinationImage);

                auto copyInfo           = VkImageCopy{};
                copyInfo.srcSubresource = ConvertSubresourceLayer(command.sourceSubresource);
                copyInfo.srcOffset      = ConvertOffset3D(command.sourceOffset);
                copyInfo.dstSubresource = ConvertSubresourceLayer(command.destinationSubresource);
                copyInfo.dstOffset      = ConvertOffset3D(command.destinationOffset);
                copyInfo.extent         = ConvertExtent3D(command.sourceSize);
                vkCmdCopyImage(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
                break;
            }
        case RHI::CopyCommandType::BufferToImage:
            {
                auto& command = _command.bufferToImage;

                auto srcBuffer = m_context->m_bufferOwner.Get(command.srcBuffer);
                auto dstImage  = m_context->m_imageOwner.Get(command.dstImage);

                auto copyInfo             = VkBufferImageCopy{};
                copyInfo.bufferOffset     = command.srcOffset;
                copyInfo.bufferRowLength  = command.srcBytesPerRow;
                // copyInfo.bufferImageHeight;
                copyInfo.imageSubresource = ConvertSubresourceLayer(command.dstSubresource);
                copyInfo.imageOffset      = ConvertOffset3D(command.dstOffset);
                copyInfo.imageExtent      = ConvertExtent3D(command.srcSize);
                vkCmdCopyBufferToImage(m_commandBuffer, srcBuffer->handle, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
                break;
            }
        case RHI::CopyCommandType::ImageToBuffer:
            {
                auto& command = _command.imageToBuffer;

                auto srcImage  = m_context->m_imageOwner.Get(command.sourceImage);
                auto dstBuffer = m_context->m_bufferOwner.Get(command.destinationBuffer);

                auto copyInfo             = VkBufferImageCopy{};
                copyInfo.bufferOffset     = command.destinationOffset;
                copyInfo.bufferRowLength  = command.destinationBytesPerRow;
                // copyInfo.bufferImageHeight;
                copyInfo.imageSubresource = ConvertSubresourceLayer(command.sourceSubresource);
                copyInfo.imageOffset      = ConvertOffset3D(command.sourceOffset);
                copyInfo.imageExtent      = ConvertExtent3D(command.sourceSize);
                vkCmdCopyImageToBuffer(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer->handle, 1, &copyInfo);
                break;
            }
        default:
            {
                RHI_UNREACHABLE();
                break;
            }
        }
    }

    void CommandList::Submit(const RHI::CommandCompute& command)
    {
        auto pipeline = m_context->m_computePipelineOwner.Get(command.pipelineState);

        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);

        if (command.BindGroups.size())
        {
            std::vector<VkDescriptorSet> descriptorSets;
            std::vector<uint32_t>        descriptorSetOffsets;

            for (auto bindGroup : command.BindGroups)
            {
                auto descriptorSet = m_context->m_bindGroupOwner.Get(bindGroup);
                descriptorSets.push_back(descriptorSet->handle);
                descriptorSetOffsets.push_back(0);
            }

            vkCmdBindDescriptorSets(
                m_commandBuffer,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                pipeline->layout,
                0,
                static_cast<uint32_t>(descriptorSets.size()),
                descriptorSets.data(),
                static_cast<uint32_t>(descriptorSetOffsets.size()),
                descriptorSetOffsets.data());
        }

        vkCmdDispatchBase(
            m_commandBuffer,
            command.parameters.offsetX,
            command.parameters.offsetY,
            command.parameters.offsetZ,
            command.parameters.countX,
            command.parameters.countY,
            command.parameters.countZ);
    }

    VkRenderingAttachmentInfo CommandList::GetAttachmentInfo(const RHI::ImagePassAttachment& passAttachment) const
    {
        auto imageView  = m_context->m_imageViewOwner.Get(passAttachment.view);
        auto attachment = passAttachment.attachment;

        auto attachmentInfo                        = VkRenderingAttachmentInfo{};
        attachmentInfo.sType                       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext                       = nullptr;
        attachmentInfo.imageView                   = imageView->handle;
        attachmentInfo.imageLayout                 = ConvertImageLayout(passAttachment.info.usage, passAttachment.info.access);
        attachmentInfo.loadOp                      = ConvertLoadOp(passAttachment.info.loadStoreOperations.loadOperation);
        attachmentInfo.storeOp                     = ConvertStoreOp(passAttachment.info.loadStoreOperations.storeOperation);
        attachmentInfo.clearValue.color.float32[0] = passAttachment.info.clearValue.color.r;
        attachmentInfo.clearValue.color.float32[1] = passAttachment.info.clearValue.color.g;
        attachmentInfo.clearValue.color.float32[2] = passAttachment.info.clearValue.color.b;
        attachmentInfo.clearValue.color.float32[3] = passAttachment.info.clearValue.color.a;
        return attachmentInfo;
    }

    std::optional<VkImageMemoryBarrier2> CommandList::TransitionResource(BarrierType barrierType, RHI::ImagePassAttachment* passAttachment) const
    {
        RHI_ASSERT(passAttachment);

        auto image = m_context->m_imageOwner.Get(passAttachment->attachment->handle);
        RHI_ASSERT(image);

        auto barrier                            = VkImageMemoryBarrier2{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.pNext                           = nullptr;
        barrier.image                           = image->handle;
        barrier.subresourceRange.aspectMask     = ConvertImageAspect(passAttachment->info.subresource.imageAspects);
        barrier.subresourceRange.baseArrayLayer = passAttachment->info.subresource.arrayBase;
        barrier.subresourceRange.layerCount     = passAttachment->info.subresource.arrayCount;
        barrier.subresourceRange.baseMipLevel   = passAttachment->info.subresource.mipBase;
        barrier.subresourceRange.levelCount     = passAttachment->info.subresource.mipCount;

        auto flags = ConvertPipelineStageAccessFlags(passAttachment->info.usage, passAttachment->stages, passAttachment->info.access);
        if (passAttachment->next)
        {
            barrier.srcQueueFamilyIndex = static_cast<Pass*>(passAttachment->pass)->m_queueFamilyIndex;
            barrier.dstQueueFamilyIndex = static_cast<Pass*>(passAttachment->next->pass)->m_queueFamilyIndex;

            barrier.srcStageMask  = flags.stages;
            barrier.srcAccessMask = flags.access;
            barrier.oldLayout     = flags.attachmentLayout;

            auto dstFlags         = ConvertPipelineStageAccessFlags(passAttachment->next->info.usage, passAttachment->next->stages, passAttachment->next->info.access);
            barrier.dstStageMask  = dstFlags.stages;
            barrier.dstAccessMask = dstFlags.access;
            barrier.newLayout     = dstFlags.attachmentLayout;
            return barrier;
        }
        else if (barrierType == BarrierType::PostPass)
        {
            if (passAttachment->attachment->swapchain)
            {
                barrier.srcStageMask  = flags.stages;
                barrier.srcAccessMask = flags.access;
                barrier.oldLayout     = flags.attachmentLayout;
                barrier.dstStageMask  = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
                barrier.dstAccessMask = 0;
                barrier.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
            else
            {
                // No passes uses the attachment anymore, no need to transition
                return std::nullopt;
            }
        }
        else if (barrierType == BarrierType::PrePass)
        {
            if (passAttachment->prev == nullptr)
            {
                barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
                barrier.srcAccessMask = 0;
                barrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.dstStageMask  = flags.stages;
                barrier.dstAccessMask = flags.access;
                barrier.newLayout     = flags.attachmentLayout;
            }
            else
            {
                // assume the previous pass transitioned into this attachment
                // no need for another transition barrier here.
                return std::nullopt;
            }
        }

        return barrier;
    }

    std::optional<VkBufferMemoryBarrier2> CommandList::TransitionResource(BarrierType barrierType, RHI::BufferPassAttachment* passAttachment) const
    {
        RHI_ASSERT(passAttachment);

        auto buffer = m_context->m_bufferOwner.Get(passAttachment->attachment->handle);
        RHI_ASSERT(buffer);

        auto barrier   = VkBufferMemoryBarrier2{};
        barrier.sType  = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        barrier.pNext  = nullptr;
        barrier.buffer = buffer->handle;
        barrier.offset = passAttachment->info.byteOffset;
        barrier.size   = passAttachment->info.byteSize;

        auto flags = ConvertPipelineStageAccessFlags(passAttachment->info.usage, passAttachment->stages, passAttachment->info.access);
        if (passAttachment->next)
        {
            barrier.srcQueueFamilyIndex = static_cast<Pass*>(passAttachment->pass)->m_queueFamilyIndex;
            barrier.dstQueueFamilyIndex = static_cast<Pass*>(passAttachment->next->pass)->m_queueFamilyIndex;

            if (barrier.srcQueueFamilyIndex == barrier.dstQueueFamilyIndex &&
                passAttachment->info.access == RHI::AttachmentAccess::Read &&
                passAttachment->next->info.access == RHI::AttachmentAccess::Read)
            {
                return std::nullopt;
            }

            barrier.srcStageMask  = flags.stages;
            barrier.srcAccessMask = flags.access;

            auto dstFlags         = ConvertPipelineStageAccessFlags(passAttachment->next->info.usage, passAttachment->next->stages, passAttachment->next->info.access);
            barrier.dstStageMask  = dstFlags.stages;
            barrier.dstAccessMask = dstFlags.access;
            return barrier;
        }
        else if (barrierType == BarrierType::PostPass)
        {
            // No passes uses the attachment anymore, no need to transition
            return std::nullopt;
        }
        else if (barrierType == BarrierType::PrePass)
        {
            if (passAttachment->prev == nullptr && passAttachment->attachment->lifetime == RHI::AttachmentLifetime::Transient)
            {
                barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
                barrier.srcAccessMask = 0;
                barrier.dstStageMask  = flags.stages;
                barrier.dstAccessMask = flags.access;
            }
            else
            {
                // assume the previous pass transitioned into this attachment
                // no need for another transition barrier here.
                return std::nullopt;
            }
        }

        return barrier;
    }

    void CommandList::TransitionPassAttachments(BarrierType barrierType, TL::Span<RHI::ImagePassAttachment*> passAttachments)
    {
        std::vector<VkImageMemoryBarrier2> barriers;

        for (auto passAttachment : passAttachments)
        {
            // only if needs to transitioned where new layout does not match the current layout
            // or a next pass depends on write operation by the current pass
            if (auto barrier = TransitionResource(barrierType, passAttachment); barrier.has_value())
            {
                barriers.push_back(barrier.value());
            }
        }

        auto dependencyInfo                    = VkDependencyInfo{};
        dependencyInfo.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext                   = nullptr;
        dependencyInfo.imageMemoryBarrierCount = barriers.size();
        dependencyInfo.pImageMemoryBarriers    = barriers.data();
        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

    void CommandList::TransitionPassAttachments(BarrierType barrierType, TL::Span<RHI::BufferPassAttachment*> passAttachments)
    {
        std::vector<VkBufferMemoryBarrier2> barriers;

        for (auto passAttachment : passAttachments)
        {
            // only if needs to transitioned where new layout does not match the current layout
            // or a next pass depends on write operation by the current pass
            if (auto barrier = TransitionResource(barrierType, passAttachment); barrier.has_value())
            {
                barriers.push_back(barrier.value());
            }
        }

        auto dependencyInfo                     = VkDependencyInfo{};
        dependencyInfo.sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext                    = nullptr;
        dependencyInfo.bufferMemoryBarrierCount = barriers.size();
        dependencyInfo.pBufferMemoryBarriers    = barriers.data();
        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

} // namespace Vulkan
