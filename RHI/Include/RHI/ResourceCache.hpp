#pragma once

#include "RHI/Resources.hpp"

namespace RHI 
{
    class ResourceCache
    {
    public:
        Handle<ImageView> CreateImageView(const ImageViewCreateInfo& createInfo);

        Handle<BufferView> CreateBufferView(const BufferViewCreateInfo& createInfo);
    
    };
}