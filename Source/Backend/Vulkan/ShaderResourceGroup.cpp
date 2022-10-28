#include "Backend/Vulkan/ShaderResourceGroup.hpp"
#include <cassert>
#include <map>
#include "RHI/Common.hpp"
#include "Backend/Vulkan/Common.hpp"
#include "Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{
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
            default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            };
        }
        else
        {
            switch (resourceType)
            {
            case EShaderInputResourceType::Image: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case EShaderInputResourceType::TexelBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            case EShaderInputResourceType::Buffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            };
        }

        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    // DescriptorSetLayout
    Result<Unique<DescriptorSetLayout>> DescriptorSetLayout::Create(const Device& device, const ShaderResourceGroupLayout& layout)
    {
        Unique<DescriptorSetLayout> descriptorSetLayout = CreateUnique<DescriptorSetLayout>(device);
        VkResult                    result              = descriptorSetLayout->Init(layout);
        return std::move(descriptorSetLayout);
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult DescriptorSetLayout::Init(const ShaderResourceGroupLayout& layout)
    {
        uint32_t bindingLocation = 0;
        for (auto& binding : layout.GetShaderInputResourceBindings())
        {
            VkDescriptorSetLayoutBinding bindingInfo = {};
            bindingInfo.binding                      = bindingLocation++;
            bindingInfo.descriptorCount              = binding.count;
            bindingInfo.descriptorType               = ConvertDescriptorType(binding.type, binding.access);
            bindingInfo.stageFlags                   = CovnertShaderStages(binding.stages);
            bindingInfo.pImmutableSamplers           = nullptr;
            m_bindings.push_back(bindingInfo);

            VkDescriptorPoolSize poolSize;
            poolSize.type            = bindingInfo.descriptorType;
            poolSize.descriptorCount = bindingInfo.descriptorCount;
            m_size.push_back(poolSize);
        }

        VkDescriptorSetLayoutCreateInfo createInfo;
        createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext        = nullptr;
        createInfo.flags        = 0;
        createInfo.bindingCount = CountElements(m_bindings);
        createInfo.pBindings    = m_bindings.data();

        return vkCreateDescriptorSetLayout(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    // DescriptorSet

    DescriptorSet::~DescriptorSet()
    {
        vkFreeDescriptorSets(m_pDevice->GetHandle(), m_pParantPool->GetHandle(), 1, &m_handle);
    }

    VkWriteDescriptorSet DescriptorSet::WriteImages(uint32_t bindingIndex, const std::vector<VkDescriptorImageInfo>& imageInfos) const
    {
        VkWriteDescriptorSet write;
        write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext           = nullptr;
        write.dstSet          = m_handle;
        write.dstBinding      = bindingIndex;
        write.dstArrayElement = 0;
        write.descriptorCount = CountElements(imageInfos);
        write.descriptorType  = m_pLayout->GetBinding(bindingIndex).descriptorType;
        write.pImageInfo      = imageInfos.data();
        return write;
    }

    VkWriteDescriptorSet DescriptorSet::WriteBuffers(uint32_t bindingIndex, const std::vector<VkDescriptorBufferInfo>& bufferInfos) const
    {
        VkWriteDescriptorSet write;
        write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext           = nullptr;
        write.dstSet          = m_handle;
        write.dstBinding      = bindingIndex;
        write.dstArrayElement = 0;
        write.descriptorCount = CountElements(bufferInfos);
        write.descriptorType  = m_pLayout->GetBinding(bindingIndex).descriptorType;
        write.pBufferInfo     = bufferInfos.data();
        return write;
    }

    VkWriteDescriptorSet DescriptorSet::WriteTexelBuffers(uint32_t bindingIndex, const std::vector<VkBufferView>& bufferViews) const
    {
        VkWriteDescriptorSet write;
        write.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext            = nullptr;
        write.dstSet           = m_handle;
        write.dstBinding       = bindingIndex;
        write.dstArrayElement  = 0;
        write.descriptorCount  = CountElements(bufferViews);
        write.descriptorType   = m_pLayout->GetBinding(bindingIndex).descriptorType;
        write.pTexelBufferView = bufferViews.data();
        return write;
    }

    // DescriptorPool

    Result<Unique<DescriptorPool>> DescriptorPool::Create(const Device& device, const Capacity& capacity)
    {
        Unique<DescriptorPool> descriptorPool = CreateUnique<DescriptorPool>(device);
        VkResult               result         = descriptorPool->Init(capacity);
        return std::move(descriptorPool);
    }

    DescriptorPool::~DescriptorPool()
    {
        vkDestroyDescriptorPool(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult DescriptorPool::Init(const Capacity& capacity)
    {
        VkDescriptorPoolCreateInfo createInfo = {};
        createInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.pNext                      = nullptr;
        createInfo.flags                      = 0;
        createInfo.maxSets                    = capacity.maxSets;
        createInfo.poolSizeCount              = CountElements(capacity.sizes);
        createInfo.pPoolSizes                 = capacity.sizes.data();

        return vkCreateDescriptorPool(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    Result<Unique<DescriptorSet>> DescriptorPool::Allocate(const DescriptorSetLayout& layout)
    {
        VkDescriptorSetLayout layoutHandle = layout.GetHandle();
        VkDescriptorSet       setHandle    = VK_NULL_HANDLE;

        VkDescriptorSetAllocateInfo allocateInfo = {};
        allocateInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.pNext                       = nullptr;
        allocateInfo.descriptorSetCount          = 1;
        allocateInfo.pSetLayouts                 = &layoutHandle;
        allocateInfo.descriptorPool              = m_handle;

        VkResult result = vkAllocateDescriptorSets(m_pDevice->GetHandle(), &allocateInfo, &setHandle);

        if (result != VK_SUCCESS)
        {
            return ResultError(result);
        }

        return std::move(CreateUnique<DescriptorSet>(*m_pDevice, layout, setHandle, *this));
    }

    // ShaderResourceGroup
    EResultCode ShaderResourceGroup::Update(const ShaderResourceGroupData& data)
    {

        std::vector<VkWriteDescriptorSet> descriptorSetWriteDescriptions = {};

        std::vector<std::vector<VkDescriptorImageInfo>> imageBindingInfos;
        imageBindingInfos.reserve(data.GetSamplersBinds().size() + data.GetImageBinds().size());

        std::vector<std::vector<VkDescriptorBufferInfo>> bufferBindingInfos;
        bufferBindingInfos.reserve(data.GetBuffersBinds().size());

        std::vector<std::vector<VkBufferView>> bufferViews;
        bufferViews.reserve(data.GetTexelBufferBinds().size());

        // Bind Samplers
        for (auto& binding : data.GetSamplersBinds())
        {
            imageBindingInfos.push_back(std::vector<VkDescriptorImageInfo>());
            auto& samplerInfos = imageBindingInfos.back();
            samplerInfos.reserve(binding.second.size());

            for (auto& sampler : binding.second)
            {
                VkDescriptorImageInfo samplerInfo = {};
                samplerInfo.imageLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
                samplerInfo.imageView             = VK_NULL_HANDLE;
                samplerInfo.sampler               = static_cast<Sampler*>(sampler)->GetHandle();
                samplerInfos.push_back(samplerInfo);
            }

            VkWriteDescriptorSet writeDesc = m_descriptorSet->WriteImages(binding.first, samplerInfos);
            descriptorSetWriteDescriptions.push_back(writeDesc);
        }

        // Bind Images
        for (auto& binding : data.GetImageBinds())
        {
            imageBindingInfos.push_back(std::vector<VkDescriptorImageInfo>());
            auto& imageInfos = imageBindingInfos.back();
            imageInfos.reserve(binding.second.size());

            for (auto& imageView : binding.second)
            {
                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageLayout           = VK_IMAGE_LAYOUT_GENERAL;
                imageInfo.imageView             = static_cast<ImageView*>(imageView)->GetHandle();
                imageInfo.sampler               = VK_NULL_HANDLE;
                imageInfos.push_back(imageInfo);
            }

            VkWriteDescriptorSet writeDesc = m_descriptorSet->WriteImages(binding.first, imageInfos);
            descriptorSetWriteDescriptions.push_back(writeDesc);
        }

        // Bind Buffers
        for (auto& binding : data.GetBuffersBinds())
        {
            bufferBindingInfos.push_back(std::vector<VkDescriptorBufferInfo>());
            auto& bufferInfos = bufferBindingInfos.back();
            bufferInfos.reserve(binding.second.size());

            for (auto& buffer : binding.second)
            {
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer                 = static_cast<Buffer*>(buffer)->GetHandle();
                bufferInfo.offset                 = 0;
                bufferInfo.range                  = buffer->GetSize();
                bufferInfos.push_back(bufferInfo);
            }

            VkWriteDescriptorSet writeDesc = m_descriptorSet->WriteBuffers(binding.first, bufferInfos);
            descriptorSetWriteDescriptions.push_back(writeDesc);
        }

        // BindTexelBuffers
        for (auto& binding : data.GetTexelBufferBinds())
        {
            bufferViews.push_back(std::vector<VkBufferView>());
            auto& bufferInfoHandles = bufferViews.back();
            bufferInfoHandles.reserve(binding.second.size());

            for (auto& bufferView : binding.second)
            {
                bufferInfoHandles.push_back(static_cast<BufferView*>(bufferView)->GetHandle());
            }

            VkWriteDescriptorSet writeDesc = m_descriptorSet->WriteTexelBuffers(binding.first, bufferInfoHandles);
            descriptorSetWriteDescriptions.push_back(writeDesc);
        }

        vkUpdateDescriptorSets(m_pDevice->GetHandle(), CountElements(descriptorSetWriteDescriptions), descriptorSetWriteDescriptions.data(), 0, nullptr);

        return EResultCode::Success;
    }

    // ShaderResourceGroupAllocator
    Expected<Unique<IShaderResourceGroup>> ShaderResourceGroupAllocator::Allocate(const ShaderResourceGroupLayout& layout)
    {

        static std::map<size_t, Unique<DescriptorSetLayout>> s_layoutCache;
        auto                                                 cacheResult = s_layoutCache.find(layout.GetHash());

        const DescriptorSetLayout* pSetLayout = nullptr;
        if (cacheResult != s_layoutCache.end())
        {
            pSetLayout = cacheResult->second.get();
        }
        else
        {
            s_layoutCache[layout.GetHash()] = CreateUnique<DescriptorSetLayout>(*m_pDevice);
            VkResult result                 = s_layoutCache[layout.GetHash()]->Init(layout);
            if (!RHI_SUCCESS(result))
            {
                return Unexpected(ConvertResult(result));
            }

            pSetLayout = s_layoutCache.find(layout.GetHash())->second.get();
        }

        const DescriptorSetLayout& descriptorSetLayout = *pSetLayout;
        Unique<DescriptorSet>      descriptorSet       = nullptr;

        for (auto& pool : m_descriptorPools)
        {
            // if ((pool->GetUsedDescriptorCount()) + 1 == pool->GetCapacity().maxSets)
            // {
            //     continue;
            // }

            auto allocationResult = pool->Allocate(descriptorSetLayout);

            if (allocationResult.has_value())
            {
                descriptorSet = std::move(allocationResult.value());
                break;
            }
            else
            {
                VkResult result = allocationResult.error();
                if (result != VK_ERROR_FRAGMENTED_POOL && result != VK_ERROR_OUT_OF_POOL_MEMORY)
                {
                    // TODO if fragmented bool and all allocated descriptorSets are unactive
                    // then Reset the pool

                    // Allocation failed unexpectedly
                    return Unexpected(ConvertResult(result));
                }
            }
        }

        // Create a new pool if all the previous pools fails to allocate.
        if (descriptorSet == nullptr)
        {
            DescriptorPool::Capacity capacity;
            capacity.maxSets = 3;
            capacity.sizes   = descriptorSetLayout.GetSize();

            Result<Unique<DescriptorPool>> result = DescriptorPool::Create(*m_pDevice, capacity);
            if (result.has_value())
            {
                m_descriptorPools.push_back(std::move(result.value()));
            }
            else
            {
                return Unexpected(ConvertResult(result.error()));
            }

            Result<Unique<DescriptorSet>> descriptorSetResult = m_descriptorPools.back()->Allocate(descriptorSetLayout);
            if (!descriptorSetResult.has_value())
            {
                return Unexpected(ConvertResult(descriptorSetResult.error()));
            }
            else
            {
                descriptorSet = std::move(descriptorSetResult.value());
            }
        }

        return CreateUnique<ShaderResourceGroup>(*m_pDevice, std::move(descriptorSet));
    }

} // namespace Vulkan
} // namespace RHI