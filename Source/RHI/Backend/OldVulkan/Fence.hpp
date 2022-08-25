#pragma once
#include "RHI/Fence.hpp"
#include "RHI/Backend/Vulkan/Device.hpp"

namespace RHI {
namespace Vulkan {

    class Fence final
        : public IFence
        , public DeviceObject<VkFence>
    {
    public:
        Fence(Device& _device)
            : DeviceObject(_device)
        {
        }
        ~Fence();
        
        VkResult Init();
        
        virtual EResultCode Wait() const override;
        virtual EResultCode Reset() const override;
        virtual EResultCode GetStatus() const override;
    };

} // namespace Vulkan
} // namespace RHI
