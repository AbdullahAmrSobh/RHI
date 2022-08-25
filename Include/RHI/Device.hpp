#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Memory.hpp"

#include "RHI/Device.hpp"

namespace RHI
{

class PhysicalDevice
{
    PhysicalDevice() = delete;

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
    bool    isDiscrete;
    EVendor vendor;
};

class IDevice
{
public:
    virtual ~IDevice() = default;
	
	inline const PhysicalDevice& GetPhysicalDevice() const;
    
    virtual void WaitIdle() const;
    
    virtual Expected<Unique<ISwapChain>> CreateSwapChain(const SwapChainDesc& desc) = 0;

    virtual Expected<Unique<IShaderProgram>> CreateShaderProgram(const ShaderProgramDesc& desc) = 0;

    virtual Expected<Unique<IPipelineState>> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) = 0;

    virtual Expected<Unique<IFence>> CreateFence() = 0;
    
    virtual Expected<Unique<ISampler>> CreateSampler(const SamplerDesc& desc) = 0;

    virtual Expected<Unique<IImage>> CreateImage(const ImageDesc& desc) = 0;

    virtual Expected<Unique<IImageView>> CreateImageView(const IImage& image, const ImageViewDesc& desc) = 0;

    virtual Expected<Unique<IBuffer>> CreateBuffer(const BufferDesc& desc) = 0;

    virtual Expected<Unique<IBufferView>> CreateBufferView(const IBuffer& buffer, const BufferViewDesc& desc) = 0;
};
using DevicePtr = Unique<IDevice>;

} // namespace RHI
