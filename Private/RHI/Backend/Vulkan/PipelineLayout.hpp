#pragma once
#include "RHI/PipelineLayout.hpp"
#include "RHI/Backend/Vulkan/Device.hpp"

namespace RHI {
namespace Vulkan {
    
    class PipelineLayout final
        : public IPipelineLayout
        , public DeviceObject<VkPipelineLayout>
    {
    public:
        PipelineLayout(Device& device)
            : DeviceObject(device)
        {
        }
        ~PipelineLayout();
        
        VkResult Init(const PipelineLayoutDesc& desc);
    };

} // namespace Vulkan
} // namespace RHI
