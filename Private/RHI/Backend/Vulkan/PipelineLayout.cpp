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

    PipelineLayout::~PipelineLayout() { vkDestroyPipelineLayout(m_pDevice->GetHandle(), m_handle, nullptr); }
    
    VkResult PipelineLayout::Init(const PipelineLayoutDesc& desc)
    {
        std::vector<VkDescriptorSetLayout> layouts       = {};
        std::vector<VkPushConstantRange>   pushConstants = {};

        VkPipelineLayoutCreateInfo createInfo = {};
        createInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.pNext                      = nullptr;
        createInfo.flags                      = 0;
        createInfo.setLayoutCount             = static_cast<uint32_t>(layouts.size());
        createInfo.pSetLayouts                = layouts.data();
        createInfo.pushConstantRangeCount     = static_cast<uint32_t>(pushConstants.size());
        createInfo.pPushConstantRanges        = pushConstants.data();

        return vkCreatePipelineLayout(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
