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

    virtual std::unique_ptr<ISwapchain> AllocateSwapChain() = 0;

    virtual std::unique_ptr<IShaderProgram> AllocateShaderProgram() = 0;

    virtual std::unique_ptr<IShaderResourceGroupAllocator> AllocateShaderResourceGroupAllocator() = 0;

    virtual std::unique_ptr<IPipelineState> AllocateGraphicsPipelineState() = 0;

    virtual std::unique_ptr<IFence> AllocateFence() = 0;

    virtual std::unique_ptr<ISampler> AllocateSampler() = 0;

    virtual std::unique_ptr<IImage> AllocateImage() = 0;

    virtual std::unique_ptr<IImageView> AllocateImageView() = 0;

    virtual std::unique_ptr<IBuffer> AllocateBuffer() = 0;

    virtual std::unique_ptr<IBufferView> AllocateBufferView() = 0;

    virtual std::unique_ptr<IFrameScheduler> AllocateFrameScheduler() = 0;
};

}  // namespace RHI