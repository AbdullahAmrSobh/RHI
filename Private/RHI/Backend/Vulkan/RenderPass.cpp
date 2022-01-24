#include "RHI/Backend/Vulkan/RenderPass.hpp"
#include "RHI/Backend/Vulkan/Utils.hpp"

namespace RHI
{
namespace Vulkan
{
    RenderPass::~RenderPass() { vkDestroyRenderPass(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult RenderPass::Init(const RenderTargetDesc& desc)
    {
        auto attachments = SetupColorAttachments(desc);

        uint32_t                           attachmentIndex = 0;
        std::vector<VkAttachmentReference> colorReferences;
        for (auto attachment : attachments)
        {
            VkAttachmentReference attachmentReference;
            attachmentReference.attachment = attachmentIndex;
            attachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            attachmentIndex++;
        }

        uint32_t depthAttachmentIndex;
        if (desc.hasDepthStencil)
        {
            auto depthAttachment        = attachments.front();
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachmentIndex        = static_cast<uint32_t>(attachments.size());
            attachments.push_back(depthAttachment);
        }

        VkAttachmentReference depthAttachmentReference;
        depthAttachmentReference.attachment = depthAttachmentIndex;
        depthAttachmentReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription    = {};
        subpassDescription.flags                   = 0;
        subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.inputAttachmentCount    = 0;
        subpassDescription.pInputAttachments       = nullptr;
        subpassDescription.colorAttachmentCount    = static_cast<uint32_t>(colorReferences.size());
        subpassDescription.pColorAttachments       = colorReferences.data();
        subpassDescription.pResolveAttachments     = nullptr;
        subpassDescription.pDepthStencilAttachment = desc.hasDepthStencil ? &depthAttachmentReference : nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments    = nullptr;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass          = 0;
        dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask       = 0;
        dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags     = 0;

        VkRenderPassCreateInfo createInfo = {};
        createInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        createInfo.pNext                  = nullptr;
        createInfo.flags                  = 0;
        createInfo.attachmentCount        = static_cast<uint32_t>(attachments.size());
        createInfo.pAttachments           = attachments.data();
        createInfo.subpassCount           = 1;
        createInfo.pSubpasses             = &subpassDescription;
        createInfo.dependencyCount        = 1;
        createInfo.pDependencies          = &dependency;
        return vkCreateRenderPass(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    RenderPass& RenderPass::FindOrCreate(const RenderTargetDesc& desc)
    {
        size_t key = 0;

        auto searchResult = RenderPass::s_RenderPasses.find(key);
        if (searchResult != RenderPass::s_RenderPasses.end())
        {
            return *searchResult->second;
        }

        RenderPass::s_RenderPasses[key] = CreateUnique<RenderPass>(*s_pDevice);
        Assert(RenderPass::s_RenderPasses[key]->Init(desc));
        return *RenderPass::s_RenderPasses[key];
    }

    std::vector<VkAttachmentDescription> RenderPass::SetupColorAttachments(const RenderTargetDesc& desc)
    {
        std::vector<VkAttachmentDescription> attachmentDescriptions;

        VkAttachmentDescription attachmentDescription = {};
        for (auto& attachment : desc.attachments)
        {
            attachmentDescription.flags          = 0;
            attachmentDescription.format         = Utils::ConvertTextureFormat(attachment.format);
            attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            
            if (attachment.usage == ERenderTargetAttacmentUsage::Present)
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            else
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            attachmentDescriptions.push_back(attachmentDescription);
        }

        return attachmentDescriptions;
    }

} // namespace Vulkan
} // namespace RHI
