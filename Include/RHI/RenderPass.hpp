#pragma once
#include "RHI/Resource.hpp"

namespace RHI
{

enum class AttachmentUsage 
{
    OutputAttachment 
};

enum class AttachmentAccess 
{
    Read,
    Write, 
};

class ImageAttachment 
{
public:
    Unique<IImage> GetImage(); 
    
private:
    Unique<IImage> m_resource; 
};

class ImagePassAttachment
{
public:
    ImagePassAttachment(const ImageAttachment& attachment);

    const ImageAttachment& GetAttachment() const;

    const IImageView& GetImageView() const;
    IImageView& GetImageView();

private:
    const ImageAttachment* m_attachment;
    Unique<IImageView> m_view;  
};

class IRenderPass
{
public:

    EResultCode UseAttachment(); 

private:
    std::vector<Unique<ImagePassAttachment>> m_outAttachments; 
    
};

}  // namespace RHI