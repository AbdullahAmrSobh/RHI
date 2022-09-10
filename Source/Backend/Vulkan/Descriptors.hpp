#pragma once
#include "RHI/Common.hpp"

#include "Backend/Vulkan/Resource.hpp"

#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace RHI
{
namespace Vulkan
{
    template <typename T>
    class DescriptorBinding
    {
    };

    using DescriptorImageBinding   = DescriptorBinding<VkImageView>;
    using DescriptorBufferBinding  = DescriptorBinding<VkBufferView>;
    using DescriptorSamplerBinding = DescriptorBinding<Sampler>;

    class DescriptorSetLayout final : public Resource<VkDescriptorSetLayout>
    {
    public:
        
    };

    class DescriptorSet final : public Resource<VkDescriptorSet>
    {
    public:
        struct WriteData
        {
            uint32_t                            binding;
            std::vector<VkDescriptorBufferInfo> bufferViewsInfo;
            std::vector<VkDescriptorImageInfo>  imageViewsInfo;
            std::vector<VkBufferView>           texelBufferViews;
        };

        void BindSampler();
        void BindBuffer();
        void BindImage();
        void BindTexelBuffer();

        VkResult CommitUpdate();

    private:
        std::vector<WriteData> m_writeData;
    };

    class DescriptorPool final : public Resource<VkDescriptorPool>
    {
    public:
        Expected<Unique<DescriptorSet>> Allocate(const DescriptorSetLayout& layout);
        void                            Free(Unique<DescriptorSet>& layout);
    };

    class DescriptorAllocator
    {
    public:
        ~DescriptorAllocator();

        Unique<DescriptorSet> Allocate(const DescriptorSetLayout& layout);

        void Free(Unique<DescriptorSet>& layout);

        void Collect();

    private:
        Device*                             m_pDevice;
        std::vector<Unique<DescriptorPool>> m_pools;
    };
    
} // namespace Vulkan
} // namespace RHI