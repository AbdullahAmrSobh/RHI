#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

    class RenderingLayoutDesc
    {

    public:
        struct SubpassDependencyElement
        {
            SubpassDependencyElement(uint32_t subpass, VkPipelineStageFlags stageMask, VkAccessFlags accessMask)
                : subpass(subpass)
                , stageMask(stageMask)
                , accessMask(accessMask)
            {
            }

            uint32_t             subpass;
            VkPipelineStageFlags stageMask;
            VkAccessFlags        accessMask;
        };
    
    public:
        RenderingLayoutDesc() = default;
        
        uint32_t BeginSubpass(VkPipelineBindPoint bindPoint);
        void EndSubpass();

        inline void UseColorAttachment(const VkAttachmentDescription& desc, VkImageLayout layout)
        {
            UseAttachment(m_colorAttachmentReferences, desc, layout);
            GetCurrentSubpass().colorAttachmentCount++;
        }
        
        inline void UseInputAttachment(const VkAttachmentDescription& desc, VkImageLayout layout)
        {
            UseAttachment(m_inputAttachmentReferences, desc, layout);
            GetCurrentSubpass().inputAttachmentCount++;
        }

        inline void SetResolveAttachment(const VkAttachmentDescription& desc, VkImageLayout layout)
        {
            UseAttachment(m_resolveAttachmentReferences, desc, layout);
            GetCurrentSubpass().pResolveAttachments = &m_resolveAttachmentReferences.back();
        }

        inline void SetDepthStencilAttachment(const VkAttachmentDescription& desc, VkImageLayout layout)
        {
            UseAttachment(m_depthStencilAttachmentReferences, desc, layout);
            GetCurrentSubpass().pDepthStencilAttachment = &m_depthStencilAttachmentReferences.back();
        }
        
        inline void AddSubpassDependency(SubpassDependencyElement src, SubpassDependencyElement dst, VkDependencyFlags flags)
        {
            m_dependencies.push_back(VkSubpassDependency());
            VkSubpassDependency& dependency = m_dependencies.back();
            dependency.srcSubpass           = src.subpass;
            dependency.dstSubpass           = dst.subpass;
            dependency.srcStageMask         = src.stageMask;
            dependency.dstStageMask         = dst.stageMask;
            dependency.srcAccessMask        = src.accessMask;
            dependency.dstAccessMask        = dst.accessMask;
            dependency.dependencyFlags      = flags;
        }

        inline VkRenderPassCreateInfo GetCreateInfo() const
        {
            VkRenderPassCreateInfo createInfo = {};
            createInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
            createInfo.pNext                  = nullptr;
            createInfo.flags                  = 0;
            createInfo.attachmentCount        = static_cast<uint32_t>(m_attachments.size());
            createInfo.pAttachments           = m_attachments.data();
            createInfo.subpassCount           = static_cast<uint32_t>(m_subpasses.size());
            createInfo.pSubpasses             = m_subpasses.data();
            createInfo.dependencyCount        = static_cast<uint32_t>(m_dependencies.size());
            createInfo.pDependencies          = m_dependencies.data();
            return createInfo;
        }

    private:
        inline VkSubpassDescription& GetCurrentSubpass()
        {
            return m_subpasses.back();
        };

        inline void UseAttachment(std::vector<VkAttachmentReference>& referenceVector, const VkAttachmentDescription& desc, VkImageLayout layout)
        {
            m_attachments.push_back(desc);
            uint32_t attachmentIndex = static_cast<uint32_t>(m_attachments.size() - 1);

            VkAttachmentReference reference;
            reference.attachment = attachmentIndex;
            reference.layout     = layout;
            referenceVector.push_back(reference);
        }

    private:
        std::vector<VkAttachmentDescription> m_attachments;
        std::vector<VkAttachmentReference>   m_inputAttachmentReferences;
        std::vector<VkAttachmentReference>   m_colorAttachmentReferences;
        std::vector<VkAttachmentReference>   m_resolveAttachmentReferences;
        std::vector<VkAttachmentReference>   m_depthStencilAttachmentReferences;
        std::vector<VkSubpassDependency>     m_dependencies;
        std::vector<VkSubpassDescription>    m_subpasses;
    };

    class RenderingLayout : public DeviceObject<VkRenderPass>
    {
    private:
        friend class Factory;

    public:
        RenderingLayout(Device& device)
            : DeviceObject(device)
        {
        }
        ~RenderingLayout();
        
        VkResult Init(const RenderingLayoutDesc& builder);
    };

} // namespace Vulkan
} // namespace RHI
