#pragma once
#include "RHI/DescriptorPool.hpp"

#include "RHI/Backend/Vulkan/DescriptorSet.hpp"
#include "RHI/Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

    class DescriptorPool final
        : public IDescriptorPool
        , public DeviceObject<VkDescriptorPool>
    {
		friend class DescriptorSet;
    public:
        DescriptorPool(Device& device)
            : DeviceObject(device)
        {
        }
        ~DescriptorPool();
        
        VkResult Init(const DescriptorPoolDesc& desc);

        virtual void             Reset() override;
        virtual DescriptorSetPtr Allocate(const IDescriptorSetLayout& layout) override;
		
    };
    using DescriptorPoolPtr = Unique<DescriptorPool>;

} // namespace Vulkan
} // namespace RHI
