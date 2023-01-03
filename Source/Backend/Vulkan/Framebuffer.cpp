#include <algorithm>

#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/Framebuffer.hpp"

#include "RHI/Attachment.hpp"

#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Image.hpp"
#include "Backend/Vulkan/RenderPass.hpp"
#include "Backend/Vulkan/Resource.hpp"

#include "Framebuffer.hpp"

namespace RHI
{
namespace Vulkan
{

Shared<RenderPassLayout> Device::CreateRenderpassLayout(std::span<const UsedImageAttachment* const> attachments)
{
    size_t key = 0;
    for (auto attachment : attachments)
    {
        key = hash_combine(attachment->GetViewHash(), key);
    }

    Shared<RenderPassLayout> layout = m_renderpassLayoutCache.Find(key);
    if (layout)
    {
        return layout;
    }

    layout = CreateShared<RenderPassLayout>(*this);
    Utils::AssertSuccess(layout->Init(attachments));
    m_renderpassLayoutCache.Insert(key, layout);
    return layout;
};

Shared<Framebuffer> Device::CreateCachedFramebuffer(std::span<UsedImageAttachment* const> attachments)
{
    auto layout = CreateRenderpassLayout(attachments);

    size_t key = 0;
    for (auto attachment : attachments)
    {
        ImageView& view = static_cast<ImageView&>(attachment->GetView());
        key             = hash_combine(attachment->GetViewHash(), hash_combine(key, std::bit_cast<size_t>(view.GetHandle())));
    }

    Shared<Framebuffer> framebuffer = m_framebufferCache.Find(key);
    if (framebuffer)
    {
        return framebuffer;
    }

    framebuffer = CreateShared<Framebuffer>(*this);
    Utils::AssertSuccess(framebuffer->Init(*layout, attachments));
    m_framebufferCache.Insert(key, framebuffer);
    return framebuffer;
};

RenderPassLayout::~RenderPassLayout()
{
    vkDestroyRenderPass(m_device->GetHandle(), m_handle, nullptr);
}

VkResult RenderPassLayout::Init(std::span<const UsedImageAttachment* const> attachments)
{
    std::vector<VkAttachmentDescription> attachmentsDescriptions = {};
    std::vector<VkAttachmentReference>   attachmentsReferences   = {};

    for (auto attachment : attachments)
    {
        VkAttachmentDescription description;
        description.flags          = 0;
        description.format         = ConvertFormat(attachment->GetViewDesc().format);
        description.samples        = VK_SAMPLE_COUNT_1_BIT;
        description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        description.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference reference;
        reference.attachment = CountElements(attachmentsDescriptions);
        reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentsDescriptions.push_back(description);
        attachmentsReferences.push_back(reference);

        RHI_WARN("Function RenderPassLayout::Init() has some hardcoded values that should be changed");
        RHI_WARN("Function RenderPassLayout::Init() also need to initalize clearValue");
    }

    VkSubpassDescription subpassDesc;
    subpassDesc.flags                   = 0;
    subpassDesc.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.inputAttachmentCount    = 0;
    subpassDesc.pInputAttachments       = nullptr;
    subpassDesc.colorAttachmentCount    = CountElements(attachmentsReferences);
    subpassDesc.pColorAttachments       = attachmentsReferences.data();
    subpassDesc.pResolveAttachments     = nullptr;
    subpassDesc.pDepthStencilAttachment = nullptr;
    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments    = nullptr;

    VkRenderPassCreateInfo createInfo {};
    createInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext           = nullptr;
    createInfo.flags           = 0;
    createInfo.attachmentCount = CountElements(attachmentsDescriptions);
    createInfo.pAttachments    = attachmentsDescriptions.data();
    createInfo.subpassCount    = 1;
    createInfo.pSubpasses      = &subpassDesc;
    createInfo.dependencyCount = 0;
    createInfo.pDependencies   = nullptr;
    return vkCreateRenderPass(m_device->GetHandle(), &createInfo, nullptr, &m_handle);
}

Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(m_device->GetHandle(), m_handle, nullptr);
}

VkResult Framebuffer::Init(const RenderPassLayout& layout, std::span<UsedImageAttachment* const> attachments)
{
    m_layout = &layout;
    m_extent = {0, 0};
    std::vector<VkImageView> handles;
    for (auto& attachment : attachments)
    {
        handles.push_back(static_cast<const ImageView&>(attachment->GetView()).GetHandle());
        m_extent.width  = std::max(attachment->GetAttachment().GetResourceDesc().extent.sizeX, m_extent.width);
        m_extent.height = std::max(attachment->GetAttachment().GetResourceDesc().extent.sizeY, m_extent.height);
        m_hash          = hash_combine(m_hash, attachment->GetViewHash());
    }

    VkFramebufferCreateInfo createInfo = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext                   = nullptr;
    createInfo.flags                   = 0;
    createInfo.renderPass              = m_layout->GetHandle();
    createInfo.attachmentCount         = CountElements(handles);
    createInfo.pAttachments            = handles.data();
    createInfo.width                   = m_extent.width;
    createInfo.height                  = m_extent.height;
    createInfo.layers                  = 1;
    return vkCreateFramebuffer(m_device->GetHandle(), &createInfo, nullptr, &m_handle);
}

}  // namespace Vulkan
}  // namespace RHI