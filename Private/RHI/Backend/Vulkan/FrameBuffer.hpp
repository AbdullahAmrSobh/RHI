#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/Image.hpp"

namespace RHI
{
namespace Vulkan
{

    class FrameBuffer : public DeviceObject<VkFramebuffer>
    {
    public:
        FrameBuffer(Device& device)
            : DeviceObject(device)
        {
        }
        ~FrameBuffer();
        
    public:
        std::vector<ImageView*> m_attachments;
    };

} // namespace Vulkan
} // namespace RHI
