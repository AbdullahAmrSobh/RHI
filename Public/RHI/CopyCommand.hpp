#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Texture.hpp"

namespace RHI {

struct BufferResourceRegion 
{
	BufferResourceRegion() = default;
	
	IBuffer* pBuffer;
	size_t offset;
	size_t range;
};

struct TextureResourceRegion 
{
	TextureResourceRegion() = default;
	
	ITextureView* pTextureView;
	Extent3D Area;
};
	
	enum class ResourceType {};

struct CopyCommand 
{
	void SetSrcResource(TextureResourceRegion src);
	void SetSrcResource(BufferResourceRegion src);
	
	void SetDstResource(TextureResourceRegion dst);
	void SetDstResource(BufferResourceRegion dst);
	
	ResourceType srcResourceType;
	union 
	{
		BufferResourceRegion source;
		TextureResourceRegion textureSource;
	};
	
	ResourceType dstResourceType;
	union 
	{
		BufferResourceRegion destination;
		TextureResourceRegion textureDestination;
	};
};

} // namespace RHI
