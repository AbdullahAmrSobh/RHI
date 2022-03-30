#pragma once
#include "RHI/DescriptorSetLayout.hpp"
#include "RHI/Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

    class Sampler;
    
    class DescriptorSetLayout final
        : public IDescriptorSetLayout
        , public DeviceObject<VkDescriptorSetLayout>
    {
	public:

        DescriptorSetLayout(Device& device)
            : DeviceObject(device)
        {
        }
        ~DescriptorSetLayout();
        
        VkResult Init(const DescriptorSeLayoutDesc& desc);
        
        inline std::vector<const Sampler*> GetSamplers() const { return m_samplers; }
    
    private:
        std::vector<const Sampler*> m_samplers;
    };
	

} // namespace Vulkan
} // namespace RHI
