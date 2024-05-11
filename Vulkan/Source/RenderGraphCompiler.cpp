#include "RenderGraphCompiler.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "CommandList.hpp"
#include "Swapchain.hpp"

namespace RHI::Vulkan
{
    struct ImageBarrierStageInfo
    {
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 access;
        VkImageLayout layout;

        inline bool operator==(ImageBarrierStageInfo other)
        {
            return stage == other.stage && access == other.access && layout == other.layout;
        }
    };

    struct BufferBarrierStageInfo
    {
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 access;

        inline bool operator==(BufferBarrierStageInfo other)
        {
            return stage == other.stage && access == other.access;
        }
    };

    inline static VkPipelineStageFlags2 GetPipelineStageFromShaderStage(Flags<ShaderStage> shader)
    {
        VkPipelineStageFlags2 flags = {};

        if (shader & ShaderStage::Compute)
            return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

        if (shader & ShaderStage::Pixel)
            flags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        if (shader & ShaderStage::Vertex)
            flags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;

        return flags;
    }

    inline static VkAccessFlags2 GetShaderAccessFlags(Access access, bool isStorage)
    {
        if (access == Access::Write)
            return isStorage ? VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT : VK_ACCESS_2_SHADER_WRITE_BIT;
        else if (access == Access::Read)
            return isStorage ? VK_ACCESS_2_SHADER_STORAGE_READ_BIT : VK_ACCESS_2_SHADER_READ_BIT;
        else if (access == Access::ReadWrite)
            return isStorage ? VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT : VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;

        return {};
    }

    inline static ImageBarrierStageInfo GetImageTransitionInfo(VkPipelineStageFlagBits2 defaultStage, const ImageAttachment* attachment, VkImageLayout defaultLayout)
    {
        if (attachment == nullptr)
        {
            return { defaultStage, VK_ACCESS_2_NONE, defaultLayout };
        }

        auto useInfo = attachment->useInfo;
        VkAccessFlags2 renderTargetAccessFlags = {};
        if (attachment->useInfo.loadStoreOperations.loadOperation == LoadOperation::Load)
            renderTargetAccessFlags |= useInfo.usage == ImageUsage::Color ? VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        if (attachment->useInfo.loadStoreOperations.storeOperation == StoreOperation::Store)
            renderTargetAccessFlags |= useInfo.usage == ImageUsage::Color ? VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        // clang-format off
        switch (useInfo.usage)
        {
        case ImageUsage::Color:           return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,    renderTargetAccessFlags,                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL         };
        case ImageUsage::Depth:           return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,       renderTargetAccessFlags,                         VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL         };
        case ImageUsage::Stencil:         return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,       renderTargetAccessFlags,                         VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL       };
        case ImageUsage::DepthStencil:    return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,       renderTargetAccessFlags,                         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        case ImageUsage::CopySrc:         return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,                   VK_ACCESS_2_TRANSFER_READ_BIT,                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL             };
        case ImageUsage::CopyDst:         return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,                   VK_ACCESS_2_TRANSFER_WRITE_BIT,                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL             };
        case ImageUsage::ShaderResource:  return { GetPipelineStageFromShaderStage(attachment->stage), GetShaderAccessFlags(attachment->access, false), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL         };
        case ImageUsage::StorageResource: return { GetPipelineStageFromShaderStage(attachment->stage), GetShaderAccessFlags(attachment->access, true),  VK_IMAGE_LAYOUT_GENERAL                          };
        default:                          RHI_UNREACHABLE(); return {};
        }
        // clang-format on
    }

    inline static BufferBarrierStageInfo GetBufferTransitionInfo(VkPipelineStageFlagBits2 defaultStage, const BufferAttachment* attachment)
    {
        if (attachment == nullptr)
        {
            return { defaultStage, VK_ACCESS_2_NONE };
        }

        // clang-format off
        switch (attachment->useInfo.usage)
        {
        case BufferUsage::Storage: return { GetPipelineStageFromShaderStage(attachment->stage),  GetShaderAccessFlags(attachment->access, false), };
        case BufferUsage::Uniform: return { GetPipelineStageFromShaderStage(attachment->stage),  GetShaderAccessFlags(attachment->access, true),  };
        case BufferUsage::Vertex:  return { VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,                VK_ACCESS_2_SHADER_READ_BIT,                     };
        case BufferUsage::Index:   return { VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT,                 VK_ACCESS_2_SHADER_READ_BIT,                     };
        case BufferUsage::CopySrc: return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,                    VK_ACCESS_2_TRANSFER_READ_BIT,                   };
        case BufferUsage::CopyDst: return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,                    VK_ACCESS_2_TRANSFER_WRITE_BIT,                  };
        default:                   RHI_UNREACHABLE(); return {};
        }
        // clang-format on
    }

    inline static VkImageMemoryBarrier2 CreateImageBarrier(VkImage image, ImageSubresourceRange subresourceRange, ImageBarrierStageInfo srcInfo, ImageBarrierStageInfo dstInfo)
    {
        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.pNext = nullptr;
        barrier.srcStageMask = srcInfo.stage;
        barrier.srcAccessMask = srcInfo.access;
        barrier.dstStageMask = dstInfo.stage;
        barrier.dstAccessMask = dstInfo.access;
        barrier.oldLayout = srcInfo.layout;
        barrier.newLayout = dstInfo.layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = ConvertSubresourceRange(subresourceRange);
        return barrier;
    }

    inline static VkBufferMemoryBarrier2 CreateBufferBarrier(VkBuffer buffer, size_t offset, size_t size, BufferBarrierStageInfo srcInfo, BufferBarrierStageInfo dstInfo)
    {
        VkBufferMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        barrier.pNext = nullptr;
        barrier.srcStageMask = srcInfo.stage;
        barrier.srcAccessMask = srcInfo.access;
        barrier.dstStageMask = dstInfo.stage;
        barrier.dstAccessMask = dstInfo.access;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = buffer;
        barrier.offset = offset;
        barrier.size = size;
        return barrier;
    }

    inline static VkSemaphoreSubmitInfo CreateSemaphoreSubmitInfo(std::pair<VkSemaphore, VkPipelineStageFlags2> pair)
    {
        VkSemaphoreSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        info.pNext = nullptr;
        info.semaphore = pair.first;
        info.value = 0;
        info.stageMask = pair.second;
        info.deviceIndex = 0;
        return info;
    }

    inline static VkCommandBufferSubmitInfo CreateCommandBufferSubmitInfo(CommandList* _commandList)
    {
        auto commandList = (ICommandList*)_commandList;
        VkCommandBufferSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        info.commandBuffer = commandList->m_commandBuffer;
        return info;
    }

    inline static TL::Vector<VkCommandBufferSubmitInfo> GetCommandBuffers(TL::Span<const CommandList*> _commandLists)
    {
        TL::Vector<VkCommandBufferSubmitInfo> submitInfos{};
        for (auto _commandList : _commandLists)
        {
            auto commandList = (ICommandList*)_commandList;
            auto submitInfo = submitInfos.emplace_back();
            submitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            submitInfo.commandBuffer = commandList->m_commandBuffer;
        }
        return submitInfos;
    }

    VkRenderingAttachmentInfo RenderGraphCompiler::GetAttachmentInfo(IContext* context, RenderGraph& renderGraph, Handle<ImageAttachment> attachmentHandle)
    {
        auto attachment = renderGraph.GetAttachment(attachmentHandle);
        auto imageViewHandle = renderGraph.GetImageView(attachmentHandle);
        auto imageView = context->m_imageViewOwner.Get(imageViewHandle);

        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = imageView->handle;
        // attachmentInfo.resolveMode;
        // attachmentInfo.resolveImageView;
        // attachmentInfo.resolveImageLayout;
        attachmentInfo.loadOp = ConvertLoadOp(attachment->useInfo.loadStoreOperations.loadOperation);
        attachmentInfo.storeOp = ConvertStoreOp(attachment->useInfo.loadStoreOperations.storeOperation);

        if (attachment->useInfo.usage == ImageUsage::Color)
        {
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            auto [r, g, b, a] = ConvertColorValue(attachment->useInfo.clearValue.f32).float32;
            attachmentInfo.clearValue.color.float32[0] = r;
            attachmentInfo.clearValue.color.float32[1] = g;
            attachmentInfo.clearValue.color.float32[2] = b;
            attachmentInfo.clearValue.color.float32[3] = a;
        }
        else
        {
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachmentInfo.clearValue.depthStencil = ConvertDepthStencilValue(attachment->useInfo.clearValue.depthStencil);
        }

        return attachmentInfo;
    }

    void RenderGraphCompiler::CompilePass(IContext* context, RenderGraph& renderGraph, Pass* pass)
    {
        ZoneScoped;

        if (pass->submitData == nullptr)
        {
            pass->submitData = new IPassSubmitData();
        }

        auto submitData = (IPassSubmitData*)pass->submitData;

        for (auto colorAttachmentHandle : pass->colorAttachments)
        {
            submitData->colorAttachments.push_back(GetAttachmentInfo(context, renderGraph, colorAttachmentHandle));
            // auto format = context->m_imageOwner.Get(renderGraph.GetAttachmentList(colorAttachmentHandle)->handle)->format;
            // submitData->colorFormats.push_back(format);
        }

        if (pass->depthStencilAttachment)
        {
            auto depthStencilAttachment = renderGraph.GetAttachment(pass->depthStencilAttachment);
            auto format = context->m_imageOwner.Get(renderGraph.GetAttachmentList(pass->depthStencilAttachment)->handle)->format;

            if (depthStencilAttachment->useInfo.usage == ImageUsage::Depth)
            {
                submitData->hasDepthAttachemnt = true;
                submitData->depthAttachmentInfo = GetAttachmentInfo(context, renderGraph, pass->depthStencilAttachment);
                submitData->depthFormat = format;
            }
            else if (depthStencilAttachment->useInfo.usage == ImageUsage::Stencil)
            {
                submitData->hasStencilAttachment = true;
                submitData->stencilAttachmentInfo = GetAttachmentInfo(context, renderGraph, pass->depthStencilAttachment);
                submitData->stencilformat = format;
            }
            else
            {
                submitData->depthAttachmentInfo = GetAttachmentInfo(context, renderGraph, pass->depthStencilAttachment);
                submitData->stencilAttachmentInfo = GetAttachmentInfo(context, renderGraph, pass->depthStencilAttachment);
                submitData->hasDepthAttachemnt = true;
                submitData->hasStencilAttachment = true;
                submitData->depthFormat = format;
                submitData->stencilformat = format;
            }
        }

        submitData->imageBarriers[BarrierType::PrePass] = GetPrepassImageBarrier(context, renderGraph, pass->imageAttachments, submitData->waitSemaphores);
        submitData->imageBarriers[BarrierType::PostPass] = GetPostpassImageBarrier(context, renderGraph, pass->imageAttachments, submitData->signalSemaphores);
        submitData->bufferBarriers[BarrierType::PrePass] = GetPrepassBufferBarrier(context, renderGraph, pass->bufferAttachments, submitData->waitSemaphores);
        submitData->bufferBarriers[BarrierType::PostPass] = GetPostpassBufferBarrier(context, renderGraph, pass->bufferAttachments, submitData->signalSemaphores);
    }

    TL::Vector<VkImageMemoryBarrier2> RenderGraphCompiler::GetPrepassImageBarrier(
        IContext* context,
        RenderGraph& renderGraph,
        TL::Span<Handle<ImageAttachment>> attachments,
        TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2>& waitSemaphores)
    {
        TL::Vector<VkImageMemoryBarrier2> barriers;

        for (auto attachmentHandle : attachments)
        {
            auto image = context->m_imageOwner.Get(renderGraph.GetImage(attachmentHandle));

            auto dstAttachment = renderGraph.m_imageAttachmentOwner.Get(attachmentHandle);
            auto srcAttachment = dstAttachment->prev != NullHandle ? renderGraph.m_imageAttachmentOwner.Get(dstAttachment->prev) : nullptr;

            auto srcInfo = GetImageTransitionInfo(VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, srcAttachment, image->initalLayout);
            auto dstInfo = GetImageTransitionInfo(VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, dstAttachment, VK_IMAGE_LAYOUT_UNDEFINED);

            if (dstInfo.layout == VK_IMAGE_LAYOUT_UNDEFINED)
                continue;

            if (srcInfo != dstInfo)
            {
                barriers.push_back(CreateImageBarrier(image->handle, dstAttachment->useInfo.subresourceRange, srcInfo, dstInfo));
            }

            if (srcAttachment == nullptr)
            {
                auto swapchain = (ISwapchain*)renderGraph.GetSwapchain(attachmentHandle);
                if (swapchain)
                {
                    waitSemaphores[swapchain->GetImageReadySemaphore()] |= srcInfo.stage;
                }
            }
        }

        return barriers;
    }

    TL::Vector<VkImageMemoryBarrier2> RenderGraphCompiler::GetPostpassImageBarrier(
        IContext* context,
        RenderGraph& renderGraph,
        TL::Span<Handle<ImageAttachment>> attachments,
        TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2>& signalSemaphores)
    {
        TL::Vector<VkImageMemoryBarrier2> barriers;

        for (auto attachmentHandle : attachments)
        {
            auto image = context->m_imageOwner.Get(renderGraph.GetImage(attachmentHandle));

            auto srcAttachment = renderGraph.m_imageAttachmentOwner.Get(attachmentHandle);
            auto dstAttachment = srcAttachment->next != NullHandle ? renderGraph.m_imageAttachmentOwner.Get(srcAttachment->next) : nullptr;

            auto srcInfo = GetImageTransitionInfo(VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, srcAttachment, VK_IMAGE_LAYOUT_UNDEFINED);
            auto dstInfo = GetImageTransitionInfo(VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, dstAttachment, VK_IMAGE_LAYOUT_UNDEFINED);

            if (dstAttachment == nullptr)
            {
                auto swapchain = (ISwapchain*)renderGraph.GetSwapchain(attachmentHandle);
                if (swapchain)
                {
                    signalSemaphores[swapchain->GetFrameReadySemaphore()] |= srcInfo.stage;
                    dstInfo.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                }
            }

            if (dstInfo.layout == VK_IMAGE_LAYOUT_UNDEFINED)
                continue;

            if (srcInfo != dstInfo)
            {
                barriers.push_back(CreateImageBarrier(image->handle, srcAttachment->useInfo.subresourceRange, srcInfo, dstInfo));
            }
        }

        return barriers;
    }

    TL::Vector<VkBufferMemoryBarrier2> RenderGraphCompiler::GetPrepassBufferBarrier(
        IContext* context,
        RenderGraph& renderGraph,
        TL::Span<Handle<BufferAttachment>> attachments,
        TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2>& waitSemaphores)
    {
        TL::Vector<VkBufferMemoryBarrier2> barriers;

        // for (auto attachmentHandle : attachments)
        // {
        //     auto buffer     = context->m_bufferOwner.Get(renderGraph.GetImage(attachmentHandle));

        // }

        (void)context;
        (void)renderGraph;
        (void)attachments;
        (void)waitSemaphores;

        return barriers;
    }

    TL::Vector<VkBufferMemoryBarrier2> RenderGraphCompiler::GetPostpassBufferBarrier(
        IContext* context,
        RenderGraph& renderGraph,
        TL::Span<Handle<BufferAttachment>> attachments,
        TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2>& signalSemaphores)
    {
        TL::Vector<VkBufferMemoryBarrier2> barriers;

        // for (auto attachmentHandle : attachments)
        // {
        // }

        (void)context;
        (void)renderGraph;
        (void)attachments;
        (void)signalSemaphores;

        return barriers;
    }
} // namespace RHI::Vulkan