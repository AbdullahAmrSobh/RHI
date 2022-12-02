#pragma once
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

class RenderPass final : public DeviceObject<VkRenderPass>
{
public:
    RenderPass(const Device& device)
        : DeviceObject(&device)
    {
    }

    ~RenderPass();

    // VkResult Init(const Pass& pass);
};

class Framebuffer final : public DeviceObject<VkFramebuffer>
{
public:
    struct AttachmentsDesc
    {
        uint32_t         colorAttachmentsCount;
        const ImageView* pColorAttachments;
        const ImageView* pDepthStencilAttachment;
    };

    Framebuffer(const Device& device)
        : DeviceObject(&device)
    {
    }
    ~Framebuffer();

    VkResult Init(VkExtent2D extent, const AttachmentsDesc& attachmentsDesc,
                  const RenderPass& renderPass);

    inline const RenderPass& GetRenderPass() const
    {
        return *m_pRenderPass;
    }

private:
    const RenderPass* m_pRenderPass;
};

}  // namespace Vulkan
}  // namespace RHI