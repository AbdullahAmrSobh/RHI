#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{
    class RenderPassBuilder
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
        RenderPassBuilder() = default;
        
        inline uint32_t BeginSubpass(VkPipelineBindPoint bindPoint)
        {
            VkSubpassDescription description    = {};
            description.flags                   = 0;
            description.pipelineBindPoint       = bindPoint;
            description.inputAttachmentCount    = 0;
            description.pInputAttachments       = nullptr;
            description.colorAttachmentCount    = 0;
            description.pColorAttachments       = nullptr;
            description.pResolveAttachments     = nullptr;
            description.pDepthStencilAttachment = nullptr;
            description.preserveAttachmentCount = 0;
            description.pPreserveAttachments    = nullptr;
            m_subpasses.push_back(description);
            return static_cast<uint32_t>(m_subpasses.size() - 1);
        }
        
        inline void EndSubpass()
        {
            VkSubpassDescription& subpass = GetCurrentSubpass();
            subpass.pColorAttachments     = (subpass.colorAttachmentCount > 0) ? m_colorAttachmentReferences.data() : nullptr;
            subpass.pInputAttachments     = (subpass.inputAttachmentCount > 0) ? m_inputAttachmentReferences.data() : nullptr;
        }
        
        inline RenderPassBuilder& UseColorAttachment(const VkAttachmentDescription& desc, VkImageLayout layout)
        {
            UseAttachment(m_colorAttachmentReferences, desc, layout);
            GetCurrentSubpass().colorAttachmentCount++;
            return *this;
        }
        
        inline RenderPassBuilder& UseInputAttachment(const VkAttachmentDescription& desc, VkImageLayout layout)
        {
            UseAttachment(m_inputAttachmentReferences, desc, layout);
            GetCurrentSubpass().inputAttachmentCount++;
            return *this;
        }
        
        inline RenderPassBuilder& SetResolveAttachment(const VkAttachmentDescription& desc, VkImageLayout layout)
        {
            UseAttachment(m_resolveAttachmentReferences, desc, layout);
            GetCurrentSubpass().pResolveAttachments = &m_resolveAttachmentReferences.back();
            return *this;
        }
        
        inline RenderPassBuilder& SetDepthStencilAttachment(const VkAttachmentDescription& desc, VkImageLayout layout)
        {
            UseAttachment(m_depthStencilAttachmentReferences, desc, layout);
            GetCurrentSubpass().pDepthStencilAttachment = &m_depthStencilAttachmentReferences.back();
            return *this;
        }
        
        inline RenderPassBuilder& AddSubpassDependency(SubpassDependencyElement src, SubpassDependencyElement dst, VkDependencyFlags flags)
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
            return *this;
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
        inline VkSubpassDescription& GetCurrentSubpass() { return m_subpasses.back(); };
        
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

} // namespace Vulkan
} // namespace RHI
