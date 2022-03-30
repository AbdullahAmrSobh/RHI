#include "RHI/Backend/Vulkan/PipelineLayout.hpp"
#include "RHI/Backend/Vulkan/DescriptorSetLayout.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

#include "RHI/Backend/Vulkan/Utils.hpp"

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
        std::vector<VkDescriptorSetLayout> layouts;
        layouts.reserve(desc.descriptorSetLayouts.size());

        std::vector<VkPushConstantRange> pushConstantRanges;
        pushConstantRanges.reserve(desc.constantBufferDescs.size());

        for (const auto& constantBuffer : desc.constantBufferDescs)
        {
            VkPushConstantRange pushConstantRange = {};
            pushConstantRange.stageFlags          = Utils::ConvertShaderStage(constantBuffer.stage);
            pushConstantRange.offset              = constantBuffer.offset;
            pushConstantRange.size                = constantBuffer.offset - constantBuffer.range;
            pushConstantRanges.push_back(pushConstantRange);
        }

        for (auto& descriptorSetLayout : desc.descriptorSetLayouts)
            layouts.push_back(static_cast<const DescriptorSetLayout*>(descriptorSetLayout)->GetHandle());

        VkPipelineLayoutCreateInfo createInfo = {};
        createInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.pNext                      = nullptr;
        createInfo.flags                      = 0;
        createInfo.setLayoutCount             = static_cast<uint32_t>(layouts.size());
        createInfo.pSetLayouts                = layouts.data();
        createInfo.pushConstantRangeCount     = static_cast<uint32_t>(pushConstantRanges.size());
        createInfo.pPushConstantRanges        = pushConstantRanges.data();

        return vkCreatePipelineLayout(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
