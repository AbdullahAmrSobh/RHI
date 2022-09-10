#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraph.hpp"
#include "RHI/Memory.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{

class IPhysicalDevice
{
    IPhysicalDevice() = delete;

public:
    enum class EVendor
    {
        Nvidia,
        Intel,
        AMD,
    };

    bool isDiscrete() const;
    bool isIntegrated() const;

    EVendor getVendor() const;

private:
    bool    m_isDiscrete;
    EVendor m_vendor;
};

struct ShaderProgramDesc;
struct SwapchainDesc;
struct GraphicsPipelineStateDesc;
struct SamplerDesc;
struct ImageDesc;
struct ImageViewDesc;
struct BufferDesc;
struct BufferViewDesc;
struct AllocationDesc;

class ISwapchain;
class IShaderProgram;
class IShaderResourceGroupAllocator;
class IPipelineState;
class ISampler;
class IImage;
class IImageView;
class IBuffer;
class IBufferView;
class IDevice;

class FrameGraph;

class IDevice
{
public:
    virtual ~IDevice() = default;

    inline const IPhysicalDevice& GetPhysicalDevice() const;

    virtual void WaitIdle() const;

    virtual Expected<Unique<ISwapchain>> CreateSwapChain(const SwapchainDesc& desc) = 0;

    virtual Expected<Unique<IShaderProgram>> CreateShaderProgram(const ShaderProgramDesc& desc) = 0;

    virtual Expected<Unique<IShaderResourceGroupAllocator>> CreateShaderResourceGroupAllocator() = 0;

    virtual Expected<Unique<IPipelineState>> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) = 0;

    virtual Expected<Unique<IFence>> CreateFence() = 0;

    virtual Expected<Unique<ISampler>> CreateSampler(const SamplerDesc& desc) = 0;

    virtual Expected<Unique<IMemoryPool>> CreateMemoryPool(const AllocationDesc& allocationDesc);

    virtual Expected<Unique<IImage>> CreateImage(const AllocationDesc& allocationDesc, const ImageDesc& desc) = 0;

    virtual Expected<Unique<IImageView>> CreateImageView(const IImage& image, const ImageViewDesc& desc) = 0;

    virtual Expected<Unique<IBuffer>> CreateBuffer(const AllocationDesc& allocationDesc, const BufferDesc& desc) = 0;

    virtual Expected<Unique<IBufferView>> CreateBufferView(const IBuffer& buffer, const BufferViewDesc& desc) = 0;

    virtual Expected<Unique<FrameGraph>> CreateFrameGraph() = 0;
};
using DevicePtr = Unique<IDevice>;

} // namespace RHI
