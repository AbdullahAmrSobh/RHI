#pragma once
#include "RHI/Attachment.hpp"

namespace RHI
{

class IDevice;
class ISwapchain;

class AttachmentsRegistry
{
public:
    AttachmentsRegistry() = default;

    void Reset();

    ResultCode ImportSwapchain(std::string name, ISwapchain& swapchain);

    ResultCode ImportImage(std::string name, IImage& image);

    const ImageAttachment* FindImageAttachment(std::string_view name) const;

    ImageAttachment* FindImageAttachment(std::string_view name);

    const ImageAttachment* FindSwapchainAttachment(std::string_view name) const;

    ImageAttachment* FindSwapchainAttachment(std::string_view name);

    std::span<ImageAttachment* const> GetImageAttachments()
    {
        return m_imageAttachments;
    }

    std::span<ImageAttachment* const> GetSwapchainAttachments()
    {
        return m_swapchainAttachments;
    }

private:
    std::unordered_map<size_t, ImageAttachment> m_attachmentsLookup;
    std::vector<ImageAttachment*>               m_imageAttachments;
    std::vector<ImageAttachment*>               m_swapchainAttachments;
};

}  // namespace RHI