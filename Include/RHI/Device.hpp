#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraph.hpp"
#include "RHI/Memory.hpp"
#include "RHI/Resource.hpp"

namespace RHI
{

class TransferQueue;

class IPhysicalDevice
{
public:
    struct Features;
    struct Properties;
    
    enum class EVendor : uint8_t
    {
        Nvidia,
        Intel,
        AMD,
    };
    
    inline bool IsDiscrete() const
    {
        return m_isDiscrete;
    }
    
    inline bool IsIntegrated() const
    {
        return !m_isDiscrete;
    }
    
    inline EVendor getVendor() const
    {
        return m_vendor;
    }

private:
    bool    m_isDiscrete;
    EVendor m_vendor;
};

class IDevice
{
public:
    virtual ~IDevice() = default;
    
    inline const IPhysicalDevice& GetPhysicalDevice() const;
    
    inline const ITransferQueue& GetTransferQueue() const;
    
    virtual EResultCode WaitIdle() const;
    
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
    
    virtual Expected<Unique<IFrameGraph>> CreateFrameGraph() = 0;
};
using DevicePtr = Unique<IDevice>;

} // namespace RHI
