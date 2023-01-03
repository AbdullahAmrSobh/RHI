#pragma once

namespace RHI
{

class ISwapchain;
class IShaderProgram;
class IShaderResourceGroupAllocator;
class IPipelineState;
class IFence;
class ISampler;
class IImage;
class IImageView;
class IBuffer;
class IBufferView;
class IFrameScheduler;

class IObjectStorageAllocator
{
public:
    virtual ~IObjectStorageAllocator() = default;

    virtual Unique<ISwapchain> AllocateSwapChain() = 0;

    virtual Unique<IShaderProgram> AllocateShaderProgram() = 0;

    virtual Unique<IShaderResourceGroupAllocator> AllocateShaderResourceGroupAllocator() = 0;

    virtual Unique<IPipelineState> AllocateGraphicsPipelineState() = 0;

    virtual Unique<IFence> AllocateFence() = 0;

    virtual Unique<ISampler> AllocateSampler() = 0;

    virtual Unique<IImage> AllocateImage() = 0;

    virtual Unique<IImageView> AllocateImageView() = 0;

    virtual Unique<IBuffer> AllocateBuffer() = 0;

    virtual Unique<IBufferView> AllocateBufferView() = 0;

    virtual Unique<IFrameScheduler> AllocateFrameScheduler() = 0;
};

}  // namespace RHI