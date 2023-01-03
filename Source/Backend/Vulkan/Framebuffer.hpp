#pragma once
#include "RHI/Attachment.hpp"

#include "Backend/Vulkan/DeviceObject.hpp"

namespace RHI
{

namespace Vulkan
{

class RenderPassLayout final : public DeviceObject<VkRenderPass>
{
public:
    RenderPassLayout(Device& device)
        : DeviceObject(device)
    {
    }

    ~RenderPassLayout();

    VkResult Init(std::span<const UsedImageAttachment* const> attachments);

public:
    std::vector<VkClearValue> m_clearValues;
};

class Framebuffer final : public DeviceObject<VkFramebuffer>
{
public:
    Framebuffer(Device& device)
        : DeviceObject(device)
    {
    }
    ~Framebuffer();

    VkResult Init(const RenderPassLayout& layout, std::span<UsedImageAttachment* const> attachments);

    const RenderPassLayout& GetLayout() const
    {
        return *m_layout;
    }

    size_t GetHash() const
    {
        return m_hash;
    }

    VkExtent2D m_extent;

private:
    size_t                  m_hash = 0;
    const RenderPassLayout* m_layout;
};

}  // namespace Vulkan
}  // namespace RHI