#include "Backend/Vulkan/ShaderResourceGroup.hpp"
#include "Backend/Vulkan/Common.hpp"

namespace RHI
{
namespace Vulkan
{
    ShaderResourceGroup::ShaderResourceGroup(const Device& device) {}

    VkDescriptorType ConvertDescriptorType(EShaderInputResourceType resourceType, EAccess access)
    {
        if (access == EAccess::Read)
        {
            switch (resourceType)
            {
            case EShaderInputResourceType::Image: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case EShaderInputResourceType::TexelBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            case EShaderInputResourceType::Buffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case EShaderInputResourceType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
            };
        }
        else
        {
            switch (resourceType)
            {
            case EShaderInputResourceType::Image: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case EShaderInputResourceType::TexelBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            case EShaderInputResourceType::Buffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            };
        }
        
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    ShaderResourceGroup::ShaderResourceGroup(const Device& device) {}

    ShaderResourceGroup::~ShaderResourceGroup() {}

    EResultCode ShaderResourceGroup::Update(const ShaderResourceGroupData& data)
    {

        VkDescriptorSet                     dstSetHandle;
        std::vector<VkWriteDescriptorSet>   writeInfos;
        std::vector<VkDescriptorImageInfo>  imageBindsInfos;
        // std::vector<VkDescriptorImageInfo>  samplerBindsInfos;
        std::vector<VkDescriptorBufferInfo> bufferInfos;

        auto getBindingIndex = [](std::string_view name)
        { return GetBindingIndex(name).or_else([](EResultCode res) { assert(res == EResultCode::Success) }).value(); };

        for (auto& bind : m_imageBinds)
        {
            writeInfos.push_back(VkWriteDescriptorSet());
            VkWriteDescriptorSet& writeInfo = writeInfos.back();

            for (auto image : bind.second)
            {
                imageBindsInfos.push_back(VkDescriptorImageInfo());
                VkDescriptorImageInfo& imageInfo = imageBindsInfos.back();
                imageInfo.sampler                = VK_NULL_HANDLE;
                imageInfo.imageView              = static_cast<ImageView*>(bind.second[0]);
                imageInfo.imageLayout            = VK_IMAGE_LAYOUT_GENERAL;
            }

            writeInfo.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext           = nullptr;
            writeInfo.dstSet          = dstSetHandle;
            writeInfo.dstBinding      = getBindingIndex(bind.first);
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = CountElements(bind.second);
            writeInfo.descriptorType  = ConvertDescriptorType(bind.type, bind.access);
            writeInfo.pImageInfo      = &imageInfo;
        }

        for (auto& bind : m_buffersBinds)
        {
            writeInfos.push_back(VkWriteDescriptorSet());
            writeInfo = writeInfos.back();

            writeInfo.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext           = nullptr;
            writeInfo.dstSet          = dstSetHandle;
            writeInfo.dstBinding      = GetBindingIndex(bind.first).or_else([](EResultCode res) { assert(res == EResultCode::Success) }).value();
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = CountElements(bind.second);
            writeInfo.descriptorType  = ConvertDescriptorType();
            writeInfo.pImageInfo;
            writeInfo.pBufferInfo;
            writeInfo.pTexelBufferView;
        }

        for (auto& bind : m_texelBufferBinds)
        {
            writeInfos.push_back(VkWriteDescriptorSet());
            writeInfo = writeInfos.back();

            writeInfo.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext           = nullptr;
            writeInfo.dstSet          = dstSetHandle;
            writeInfo.dstBinding      = GetBindingIndex(bind.first).or_else([](EResultCode res) { assert(res == EResultCode::Success) }).value();
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = CountElements(bind.second);
            writeInfo.descriptorType  = ConvertDescriptorType();
            writeInfo.pImageInfo;
            writeInfo.pBufferInfo;
            writeInfo.pTexelBufferView;
        }

        for (auto& bind : m_samplersBinds)
        {
            writeInfos.push_back(VkWriteDescriptorSet());
            writeInfo = writeInfos.back();

            writeInfo.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext           = nullptr;
            writeInfo.dstSet          = dstSetHandle;
            writeInfo.dstBinding      = GetBindingIndex(bind.first).or_else([](EResultCode res) { assert(res == EResultCode::Success) }).value();
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = CountElements(bind.second);
            writeInfo.descriptorType  = ConvertDescriptorType();
            writeInfo.pImageInfo;
            writeInfo.pBufferInfo;
            writeInfo.pTexelBufferView;
        }

        for (auto& bind : m_constants) {}

        vkUpdateDescriptorSets(m_pDevice->GetHandle(), CountElements(writeInfos), writeInfos.data(), 0, nullptr);
    }

    ShaderResourceGroupAllocator::ShaderResourceGroupAllocator(const Device& device) {}

    ShaderResourceGroupAllocator::~ShaderResourceGroupAllocator() {}
    
    Expected<Unique<IShaderResourceGroup>> ShaderResourceGroupAllocator::Allocate() const {}
    
    void ShaderResourceGroupAllocator::Free(Unique<IShaderResourceGroup> group) {}

} // namespace Vulkan
} // namespace RHI