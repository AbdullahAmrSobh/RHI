#pragma once
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

class ImagePassAttachment;

class RenderPassLayout final : public DeviceObject<VkRenderPass>
{
public:
    RenderPassLayout(const Device& device)
        : DeviceObject(&device)
    {
    }

    ~RenderPassLayout();

    VkResult Init(const std::span<const ImagePassAttachment*> passAttachment);
};

class Framebuffer final : public DeviceObject<VkFramebuffer>
{
public:
    Framebuffer(const Device& device)
        : DeviceObject(&device)
    {
    }
    ~Framebuffer();

    VkResult Init(const RenderPassLayout&     layout,
                  std::span<ImageView* const> attachments);

private:
    const RenderPassLayout* m_layout;
    
};

}  // namespace Vulkan
}  // namespace RHI