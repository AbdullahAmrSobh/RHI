#include "RHI/Pch.hpp"

#include "RHI/Common.hpp"

#include "RHI/AttachmentsRegistry.hpp"

#include "RHI/Debug.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{

void AttachmentsRegistry::Reset()
{
    m_attachmentsLookup.clear();
    m_imageAttachments.clear();
}

ResultCode AttachmentsRegistry::ImportSwapchain(std::string name, ISwapchain& swapchain)
{
    size_t key = std::hash<std::string> {}(name);

    if (m_attachmentsLookup.find(key) != m_attachmentsLookup.end())
    {
        RHI_WARN("Trying to import resource with a name, that already exists in the registry. ");
        return ResultCode::InvalidArguments;
    }

    ImageAttachment& attachment = m_attachmentsLookup[key] = ImageAttachment(name, swapchain);
    m_imageAttachments.push_back(&attachment);
    m_swapchainAttachments.push_back(&attachment);
    return ResultCode::Success;
}

ResultCode AttachmentsRegistry::ImportImage(std::string name, IImage& image)
{
    size_t key = std::hash<std::string> {}(name);

    if (m_attachmentsLookup.find(key) != m_attachmentsLookup.end())
    {
        RHI_WARN("Trying to import resource with a name, that already exists in the registry. ");
        return ResultCode::InvalidArguments;
    }

    ImageAttachment& attachment = m_attachmentsLookup[key] = ImageAttachment(name, image);
    m_imageAttachments.push_back(&attachment);
    return ResultCode::Success;
}

ImageAttachment* AttachmentsRegistry::FindImageAttachment(std::string_view name)
{
    size_t      key = std::hash<std::string_view> {}(name);
    const auto& it  = m_attachmentsLookup.find(key);

    if (it != m_attachmentsLookup.end())
    {
        return &it->second;
    }

    return nullptr;
}

ImageAttachment* AttachmentsRegistry::FindSwapchainAttachment(std::string_view name)
{
    size_t      key = std::hash<std::string_view> {}(name);
    const auto& it  = m_attachmentsLookup.find(key);

    if (it != m_attachmentsLookup.end())
    {
        return &it->second;
    }

    return nullptr;
}

}  // namespace RHI