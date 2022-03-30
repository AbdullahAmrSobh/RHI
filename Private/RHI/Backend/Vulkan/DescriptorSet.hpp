#pragma once
#include "RHI/DescriptorSet.hpp"

#include "RHI/Backend/Vulkan/DescriptorSetLayout.hpp"

#include "RHI/Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

    class DescriptorPool;

    class DescriptorSet final
        : public IDescriptorSet
        , public DeviceObject<VkDescriptorSet>
    {
    public:
        DescriptorSet(Device& device, DescriptorPool& pool, const DescriptorSetLayout& layout)
            : DeviceObject(device)
			, m_pParentPool(&pool)
			, m_pLayout(&layout)
		{
        }
        ~DescriptorSet();
        
        VkResult Init(const DescriptorSetLayout& layout);
        
        virtual void CommitUpdates() override;
    
    private:
        const DescriptorSetLayout* m_pLayout;
        const DescriptorPool*      m_pParentPool;
    };

} // namespace Vulkan
} // namespace RHI
