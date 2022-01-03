#pragma once
#include "RHI/Common.hpp"
#include "RHI/Definitions.hpp"
#include "RHI/Texture.hpp"

namespace RHI
{

struct RenderTargetLayout
{
    std::vector<EPixelFormat> colorFormats;
    EPixelFormat              depthFormat;
};

struct RenderTargetDesc
{
    uint32_t      colorAttachmentCount;
    ITextureView* pColorAttachments;
    ITextureView* pDepthAttachment;
	Extent2D 	  extent;
};

class IRenderTarget
{
public:
	virtual ~IRenderTarget() = default;
	
protected:
	RenderTargetDesc m_desc;

};

using RenderTargetPtr = Unique<IRenderTarget>;

} // namespace RHI
