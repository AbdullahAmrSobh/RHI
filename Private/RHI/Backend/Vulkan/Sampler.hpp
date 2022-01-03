#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Sampler.hpp"

namespace RHI
{
namespace Vulkan
{

    class Sampler final
        : public ISampler
        , public DeviceObject<VkSampler>
    {
    public:
        Sampler(Device& device)
            : DeviceObject(device)
        {
        }
        ~Sampler();
        
        VkResult Init(const SamplerDesc& desc);
    };

} // namespace Vulkan
} // namespace RHI
