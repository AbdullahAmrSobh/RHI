#pragma once
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

class FramebufferLayout final : public DeviceObject<VkRenderPass>
{
public:
    RenderPass(const Device& device)
        : DeviceObject(&device)
    {
    }

    ~RenderPass();
};

class Framebuffer final : public DeviceObject<VkFramebuffer>
{
public:
    Framebuffer(const Device& device)
        : DeviceObject(&device)
    {
    }
    ~Framebuffer();

    VkResult Init(VkExtent2D extent, const AttachmentsDesc& attachmentsDesc,
                  const RenderPass& renderPass);
};

}  // namespace Vulkan
}  // namespace RHI