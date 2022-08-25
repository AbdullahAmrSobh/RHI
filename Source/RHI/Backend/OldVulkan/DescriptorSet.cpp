#include "RHI/Backend/Vulkan/DescriptorSet.hpp"
#include "RHI/Backend/Vulkan/DescriptorPool.hpp"
#include "RHI/Backend/Vulkan/DescriptorSetLayout.hpp"

#include "RHI/Backend/Vulkan/Buffer.hpp"
#include "RHI/Backend/Vulkan/Sampler.hpp"
#include "RHI/Backend/Vulkan/Image.hpp"

namespace RHI
{
namespace Vulkan
{
	
    DescriptorSet::~DescriptorSet() { vkFreeDescriptorSets(m_pDevice->GetHandle(), m_pParentPool->GetHandle(), 1, &m_handle); }

    VkResult DescriptorSet::Init(const DescriptorSetLayout& layout)
    {
        VkDescriptorSetLayout       layoutHandle = layout.GetHandle();
        VkDescriptorSetAllocateInfo allocateInfo = {};
        allocateInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pNext                       = nullptr;
        allocateInfo.descriptorPool              = m_pParentPool->GetHandle();
        allocateInfo.descriptorSetCount          = 1;
        allocateInfo.pSetLayouts                 = &layoutHandle;
        return vkAllocateDescriptorSets(m_pDevice->GetHandle(), &allocateInfo, &m_handle);
    }

    void DescriptorSet::CommitUpdates()
    {
        std::vector<VkDescriptorImageInfo>  imagesInfos;
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkWriteDescriptorSet>   writeDescriptorSets;

        for (auto& binding : m_bindings)
        {
            VkWriteDescriptorSet writeDescriptorSet = {};
            writeDescriptorSet.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet               = m_handle;
            writeDescriptorSet.dstBinding           = binding.descriptorReference.binding;
            writeDescriptorSet.dstArrayElement      = binding.descriptorReference.binding;
            writeDescriptorSet.descriptorCount      = binding.elementsCount;

            switch (binding.type)
            {
            case EDescriptorType::Sampler:
            {
                for (uint32_t i = 0; i < binding.elementsCount; ++i)
                {
                    imagesInfos.push_back(VkDescriptorImageInfo());
                    auto& info                    = imagesInfos.back();
                    info.sampler                  = static_cast<const Sampler&>(binding.pSamplers[i]).GetHandle();
                    info.imageView                = VK_NULL_HANDLE;
                    info.imageLayout              = VK_IMAGE_LAYOUT_MAX_ENUM;
                    writeDescriptorSet.pImageInfo = &info;
                }

                writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            }
            break;
            case EDescriptorType::TexelBuffer:
            {
                switch (binding.accessType)
                {
                case RHI::EDescriptorAccessType::ReadOnly: writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; break;
                case RHI::EDescriptorAccessType::Unoredered: writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER; break;
                default: writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM; break;
                };

                for (uint32_t i = 0; i < binding.elementsCount; ++i)
                {
                    imagesInfos.push_back(VkDescriptorImageInfo());
                    auto& info                    = imagesInfos.back();
                    info.sampler                  = static_cast<const Sampler&>(binding.pSamplers[i]).GetHandle();
                    info.imageView                = VK_NULL_HANDLE;
                    info.imageLayout              = VK_IMAGE_LAYOUT_MAX_ENUM;
                    writeDescriptorSet.pImageInfo = &info;
                }
            }
            break;
            case EDescriptorType::Image:
            {
                switch (binding.accessType)
                {
                case RHI::EDescriptorAccessType::ReadOnly: writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
                case RHI::EDescriptorAccessType::Unoredered: writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
                default: writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM; break;
                };
                
                for (uint32_t i = 0; i < binding.elementsCount; ++i)
                {
                    imagesInfos.push_back(VkDescriptorImageInfo());
                    auto& info                    = imagesInfos.back();
                    info.imageView                = static_cast<const ImageView&>(*binding.pImageBindings->pImageView).GetHandle();
                    info.sampler                  = VK_NULL_HANDLE;
                    info.imageLayout              = static_cast<VkImageLayout>(static_cast<uint32_t>(binding.pImageBindings->layout));
                    writeDescriptorSet.pImageInfo = &info;
                }
            }
            break;
            case EDescriptorType::UniformBuffer:
            {
                switch (binding.accessType)
                {
                case RHI::EDescriptorAccessType::ReadOnly: writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
                case RHI::EDescriptorAccessType::Unoredered: writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
                default: writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM; break;
                };

                for (uint32_t i = 0; i < binding.elementsCount; ++i)
                {
                    bufferInfos.push_back(VkDescriptorBufferInfo());
                    auto& info                     = bufferInfos.back();
                    info.buffer                    = static_cast<const Buffer&>(*binding.pBufferBindings->pBuffer).GetHandle();
                    info.offset                    = binding.pBufferBindings->offset;
                    info.range                     = binding.pBufferBindings->range;
                    writeDescriptorSet.pBufferInfo = &info;
                }
            }
            break;
            default:
                assert(false); // unexpected error.
                break;
            };

            writeDescriptorSets.push_back(writeDescriptorSet);
        }
		// TODO
        std::vector<VkCopyDescriptorSet> copyDescriptorSets;
        
        vkUpdateDescriptorSets(m_pDevice->GetHandle(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(),
                               static_cast<uint32_t>(copyDescriptorSets.size()), copyDescriptorSets.data());
    }

} // namespace Vulkan
} // namespace RHI
