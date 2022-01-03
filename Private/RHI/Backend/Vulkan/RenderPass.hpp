#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"


namespace RHI {
namespace Vulkan {

    
    class RenderPass : public DeviceObject<VkRenderPass>
    {
    public:
        RenderPass(Device& device)
            : DeviceObject(device)
        {
        }
        ~RenderPass();
        
        size_t GetHash() const;
    };
    using RenderPassRef = Shared<RenderPass>;

}
}
