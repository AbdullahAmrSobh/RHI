#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/RenderPass.hpp"
#include "RHI/Backend/Vulkan/Texture.hpp"

#include "RHI/RenderTarget.hpp"

namespace RHI
{
namespace Vulkan
{

    class FrameBuffer final
        : public IRenderTarget
        , public DeviceObject<VkFramebuffer>
    {
    public:

        FrameBuffer(Device& device)
            : DeviceObject(device)
        {
        }
        ~FrameBuffer();

        VkResult Init(const RenderTargetDesc& desc);

        virtual const ArrayView<const ITextureView*> GetAttachments() const override;

    public:
        std::vector<TextureView*> m_attachments;
    };

} // namespace Vulkan
} // namespace RHI
