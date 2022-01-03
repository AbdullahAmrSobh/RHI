#include "RHI/Backend/Vulkan/PipelineLayout.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

namespace RHI
{
namespace Vulkan
{
    Expected<PipelineLayoutPtr> Factory::CreatePipelineLayout(const PipelineLayoutDesc& desc)
    {
        auto     pipelineLayout = CreateUnique<PipelineLayout>(*m_device);
        VkResult result         = pipelineLayout->Init(desc);
        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));
        
        return pipelineLayout;
    }

	PipelineLayout::~PipelineLayout() 
	{
		vkDestroyPipelineLayout(m_pDevice->GetHandle(), m_handle, nullptr);
	}
	
	VkResult PipelineLayout::Init(const PipelineLayoutDesc& desc) 
	{
		VkPipelineLayoutCreateInfo createInfo = {};
		
		return vkCreatePipelineLayout(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
	}

} // namespace Vulkan
} // namespace RHI
