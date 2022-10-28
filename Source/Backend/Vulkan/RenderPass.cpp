#include "RHI/FrameGraphAttachment.hpp"
#include "RHI/FrameGraphPass.hpp"

#include "Backend/Vulkan//Device.hpp"
#include "Backend/Vulkan//RenderPass.hpp"
#include "Backend/Vulkan/Commands.hpp"

namespace RHI
{
namespace Vulkan
{

    VkAttachmentLoadOp ConvertLoadOp(EAttachmentLoadOp loadOp)
    {
        switch (loadOp)
        {
        case EAttachmentLoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
        case EAttachmentLoadOp::Discard: return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case EAttachmentLoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
    }

    VkAttachmentStoreOp ConvertStoreOp(EAttachmentStoreOp storeOp)
    {
        switch (storeOp)
        {
        case RHI::EAttachmentStoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
        case RHI::EAttachmentStoreOp::Discard: return VK_ATTACHMENT_STORE_OP_NONE;
        case RHI::EAttachmentStoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
        return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
    }

    VkImageLayout GetAttachmentOptimalLayout(const ImagePassAttachment* pImageAttachment)
    {
        if (pImageAttachment != nullptr)
        {
            // TODO Deduce the optimal layout based on the usage and the access of the prev attachment.

            auto readOnlyLookup = {
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_PREINITIALIZED,
                VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            };

            auto readWriteLookup = {
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            };

            // switch (pImageAttachment->GetAccess())
            // {
            // case EAccess::Read:
            // case EAccess::ReadWrite:
            // };
        }

        return VK_IMAGE_LAYOUT_UNDEFINED;
    }

    Result<Unique<RenderPass>> RenderPass::Create(const Device& device, const Pass& pass)
    {
        Unique<RenderPass> renderPass = CreateUnique<RenderPass>(device);
        VkResult           result     = renderPass->Init(pass);
        if (!RHI_SUCCESS(result))
        {
            return ResultError(result);
        }
        return std::move(renderPass);
    }

    RenderPass::~RenderPass()
    {
        vkDestroyRenderPass(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult RenderPass::Init(const Pass& pass)
    {
        const std::vector<ImagePassAttachment*>& passAttachments = pass.GetImagePassAttachments();

        uint32_t currentAttachmentIndex = 0;

        std::vector<VkAttachmentDescription> attachmentsDescriptions;
        // The + 1 is to account for the depth stencil attachment;
        attachmentsDescriptions.reserve(CountElements(pass.GetImagePassAttachments()) + 1);

        std::vector<VkAttachmentReference> inputAttachments;
        std::vector<VkAttachmentReference> colorAttachments;
        VkAttachmentReference              depthStencilAttachment;

        std::vector<uint32_t> preserveAttachments;
        for (auto& attachment : passAttachments)
        {
            VkAttachmentDescription attachmentDescription{};
            attachmentDescription.flags          = 0;
            attachmentDescription.format         = ConvertFormat(attachment->GetDesc().format);
            attachmentDescription.samples        = ConvertSampleCount(attachment->GetSampleCount());
            attachmentDescription.loadOp         = ConvertLoadOp(attachment->GetLoadStoreOp().loadOp);
            attachmentDescription.storeOp        = ConvertStoreOp(attachment->GetLoadStoreOp().storeOp);
            attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout  = GetAttachmentOptimalLayout(attachment->GetPerv());
            attachmentDescription.finalLayout    = GetAttachmentOptimalLayout(attachment->GetNext());

            attachmentsDescriptions.push_back(attachmentDescription);

            VkAttachmentReference reference = {};
            reference.attachment            = currentAttachmentIndex++;
            reference.layout                = GetAttachmentOptimalLayout(attachment);

            if (attachment->GetUsage() == EAttachmentUsage::RenderTarget)
            {
                colorAttachments.push_back(reference);
            }
            else if (attachment->GetUsage() == EAttachmentUsage::DepthStencil)
            {
                inputAttachments.push_back(reference);
            }
            else if (attachment->GetUsage() == EAttachmentUsage::DepthStencil)
            {
                depthStencilAttachment = reference;
            }
        }

        VkSubpassDescription subpassDescription{};
        subpassDescription.flags = 0;
        switch (pass.GetQueueType())
        {
        case EHardwareQueueType::Graphics: subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        case EHardwareQueueType::Compute: subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        default: assert(pass.GetQueueType() != EHardwareQueueType::Transfer);
        };
        subpassDescription.inputAttachmentCount    = CountElements(inputAttachments);
        subpassDescription.pInputAttachments       = inputAttachments.data();
        subpassDescription.colorAttachmentCount    = CountElements(colorAttachments);
        subpassDescription.pColorAttachments       = colorAttachments.data();
        subpassDescription.pResolveAttachments     = nullptr;
        subpassDescription.pDepthStencilAttachment = nullptr;
        subpassDescription.preserveAttachmentCount = CountElements(preserveAttachments);
        subpassDescription.pPreserveAttachments    = preserveAttachments.data();

        if (pass.HasDepthStencil())
        {
            const ImagePassAttachment* attachment = pass.GetDepthStencilAttachment();

            VkAttachmentDescription attachmentDescription = {};
            attachmentDescription.flags                   = {};
            attachmentDescription.format                  = ConvertFormat(attachment->GetDesc().format);
            attachmentDescription.samples                 = ConvertSampleCount(attachment->GetSampleCount());
            attachmentDescription.loadOp                  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.storeOp                 = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.stencilLoadOp           = ConvertLoadOp(attachment->GetLoadStoreOp().loadOp);
            attachmentDescription.stencilStoreOp          = ConvertStoreOp(attachment->GetLoadStoreOp().storeOp);

            attachmentsDescriptions.push_back(attachmentDescription);

            subpassDescription.pDepthStencilAttachment = &depthStencilAttachment;
        }

        VkRenderPassCreateInfo createInfo;
        createInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext           = nullptr;
        createInfo.flags           = 0;
        createInfo.attachmentCount = CountElements(attachmentsDescriptions);
        createInfo.pAttachments    = attachmentsDescriptions.data();
        createInfo.subpassCount    = 1;
        createInfo.pSubpasses      = &subpassDescription;
        createInfo.dependencyCount = 0;
        createInfo.pDependencies   = nullptr;

        return vkCreateRenderPass(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    EResultCode Pass::Submit()
    {
        const Queue* pQueue = nullptr;
        if (m_queueType == EHardwareQueueType::Graphics)
        {
            pQueue = &m_pDevice->GetGraphicsQueue();
        }
        else
        {
            pQueue = &m_pDevice->GetComputeQueue();
        }

        VkCommandBufferSubmitInfo cmdBufSubmitInfo = {};
        cmdBufSubmitInfo.sType                     = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdBufSubmitInfo.pNext                     = nullptr;
        cmdBufSubmitInfo.deviceMask                = UINT32_MAX;
        cmdBufSubmitInfo.commandBuffer             = m_commandBuffers[m_currentBackbufferIndex++]->GetHandle();

        Queue::SubmitRequest submitReq{};
        submitReq.commandBuffers.push_back(cmdBufSubmitInfo);

        for (auto& waitSemaphore : m_pWaitSemaphores)
        {
            VkSemaphoreSubmitInfo semaphoreSubmitInfo = {};
            semaphoreSubmitInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            semaphoreSubmitInfo.pNext                 = nullptr;
            semaphoreSubmitInfo.semaphore             = waitSemaphore->GetHandle();
            semaphoreSubmitInfo.value                 = {};
            semaphoreSubmitInfo.stageMask             = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
            semaphoreSubmitInfo.deviceIndex           = 0;
            submitReq.waitSemaphores.push_back(semaphoreSubmitInfo);
        }

        VkSemaphoreSubmitInfo semaphoreSubmitInfo = {};
        semaphoreSubmitInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        semaphoreSubmitInfo.pNext                 = nullptr;
        semaphoreSubmitInfo.semaphore             = m_signalSemaphore->GetHandle();
        semaphoreSubmitInfo.value                 = {};
        semaphoreSubmitInfo.stageMask             = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        semaphoreSubmitInfo.deviceIndex           = 0;

        // signal passFinishedSemaphore.
        submitReq.signalSemaphores.push_back(semaphoreSubmitInfo);

        std::vector<Queue::SubmitRequest> submitRequests{submitReq};
        VkResult                          result = pQueue->Submit(submitRequests, static_cast<Fence*>(m_pSignalFences));
        return ConvertResult(result);
    }

    Result<Unique<Framebuffer>> Framebuffer::Create(Device& device, VkExtent2D extent, const AttachmentsDesc& attachments, const RenderPass& renderPass)
    {
        Unique<Framebuffer> framebuffer = CreateUnique<Framebuffer>(device);
        VkResult            result      = framebuffer->Init(extent, attachments, renderPass);
        if (RHI_SUCCESS(result))
        {
            return std::move(framebuffer);
        }
        return ResultError(result);
    }

    Framebuffer::~Framebuffer()
    {
        vkDestroyFramebuffer(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult Framebuffer::Init(VkExtent2D extent, const AttachmentsDesc& attachmentsDesc, const RenderPass& renderPass)
    {
        std::vector<VkImageView> attachments;

        for (uint32_t index = 0; index < attachmentsDesc.colorAttachmentsCount; index++)
        {
            const ImageView& attachment = attachmentsDesc.pColorAttachments[index];
            attachments.push_back(attachment.GetHandle());
        }

        if (attachmentsDesc.pDepthStencilAttachment != nullptr)
        {
            attachments.push_back(attachmentsDesc.pDepthStencilAttachment->GetHandle());
        }

        VkFramebufferCreateInfo createInfo{};
        createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext           = nullptr;
        createInfo.flags           = 0;
        createInfo.renderPass      = m_pRenderPass->GetHandle();
        createInfo.attachmentCount = CountElements(attachments);
        createInfo.pAttachments    = attachments.data();
        createInfo.width           = extent.width;
        createInfo.height          = extent.height;
        createInfo.layers          = 1;

        return vkCreateFramebuffer(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    EResultCode Pass::Compile()
    {
        FrameGraphBuilder builder(m_pFrameGraph);
        m_pCallback->Setup(builder);
    }k

} // namespace Vulkan
} // namespace RHI