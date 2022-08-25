#include "RHI/Backend/Vulkan/RenderingLayout.hpp"

namespace RHI
{
namespace Vulkan
{

    uint32_t RenderingLayoutDesc::BeginSubpass(VkPipelineBindPoint bindPoint)
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

    void RenderingLayoutDesc::EndSubpass()
    {
        VkSubpassDescription& subpass = GetCurrentSubpass();
        subpass.pColorAttachments     = (subpass.colorAttachmentCount > 0) ? m_colorAttachmentReferences.data() : nullptr;
        subpass.pInputAttachments     = (subpass.inputAttachmentCount > 0) ? m_inputAttachmentReferences.data() : nullptr;
    }

} // namespace Vulkan
} // namespace RHI
