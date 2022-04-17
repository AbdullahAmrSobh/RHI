#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Image.hpp"

namespace RHI
{

struct BufferResourceRegion
{
    BufferResourceRegion() = default;

    IBuffer* pBuffer;
    size_t   offset;
    size_t   range;
};

struct ImageResourceRegion
{
    ImageResourceRegion() = default;

    IImageView*           pImageView;
    Extent3D              area;
    ImageSubresourceRange subresourceRange;
};

struct CopyCommand
{
	CopyCommand()
		: nullSource(nullptr)
		, nullDestination(nullptr)
	{
	}

    void SetCopySourceResource(ImageResourceRegion source);
    void SetCopySourceResource(BufferResourceRegion source);
    
    void SetCopyDestinationResource(ImageResourceRegion destination);
    void SetCopyDestinationResource(BufferResourceRegion destination);
    
    EResourceType sourceResourceType;
    union
    {
        nullptr_t            nullSource;
        BufferResourceRegion sourceBuffer;
        ImageResourceRegion  sourceImage;
    };
    
    EResourceType destinationResourceType;
    union
    {
        nullptr_t            nullDestination;
        BufferResourceRegion destinationBuffer;
        ImageResourceRegion  destinationImage;
    };
};

} // namespace RHI
