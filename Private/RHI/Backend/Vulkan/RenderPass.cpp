#include "RHI/Backend/Vulkan/RenderPassBuilder.hpp"
#include "RHI/Backend/Vulkan/RenderPass.hpp"
#include "RHI/Backend/Vulkan/Utils.hpp"

namespace RHI
{
namespace Vulkan
{
    RenderPass::~RenderPass() { vkDestroyRenderPass(m_pDevice->GetHandle(), m_handle, nullptr); }
    
    VkResult RenderPass::Init(const RenderPassBuilder& builder)
    {
		VkRenderPassCreateInfo createInfo = builder.GetCreateInfo(); 
        return vkCreateRenderPass(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
