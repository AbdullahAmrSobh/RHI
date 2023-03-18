#pragma once
#include "RHI/Common.hpp"

namespace RHI
{

struct SwapchainDesc;
struct ShaderProgramDesc;
struct GraphicsPipelineStateDesc;
struct SamplerDesc;
struct AllocationDesc;
struct ImageDesc;
struct ImageViewDesc;
struct BufferDesc;
struct BufferViewDesc;

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

class IPhysicalDevice
{
public:
    enum class Vendor
    {
        Nvidia,
        Intel,
        AMD,
    };

    bool   m_isDiscrete;
    Vendor m_vendor;
};

class IDevice
{
public:
    IDevice(const IPhysicalDevice& physicalDevice)
        : m_physicalDevice(&physicalDevice)
    {
    }
    virtual ~IDevice() = default;

    const IPhysicalDevice& GetPhysicalDevice() const
    {
        return *m_physicalDevice;
    }

    virtual ResultCode WaitIdle() const = 0;

    virtual Expected<std::unique_ptr<ISwapchain>> CreateSwapChain(const SwapchainDesc& desc) = 0;

    virtual Expected<std::unique_ptr<IShaderProgram>> CreateShaderProgram(const ShaderProgramDesc& desc) = 0;

    virtual Expected<std::unique_ptr<IShaderResourceGroupAllocator>> CreateShaderResourceGroupAllocator() = 0;

    virtual Expected<std::unique_ptr<IPipelineState>> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) = 0;

    virtual Expected<std::unique_ptr<IFence>> CreateFence() = 0;

    virtual Expected<std::unique_ptr<ISampler>> CreateSampler(const SamplerDesc& desc) = 0;

    virtual Expected<std::unique_ptr<IImage>> CreateImage(const AllocationDesc& allocationDesc, const ImageDesc& desc) = 0;

    virtual Expected<std::unique_ptr<IImageView>> CreateImageView(const IImage& image, const ImageViewDesc& desc) = 0;

    virtual Expected<std::unique_ptr<IBuffer>> CreateBuffer(const AllocationDesc& allocationDesc, const BufferDesc& desc) = 0;

    virtual Expected<std::unique_ptr<IBufferView>> CreateBufferView(const IBuffer& buffer, const BufferViewDesc& desc) = 0;

    virtual Expected<std::unique_ptr<IFrameScheduler>> CreateFrameScheduler() = 0;

private:
    const IPhysicalDevice* m_physicalDevice;
};

}  // namespace RHI
