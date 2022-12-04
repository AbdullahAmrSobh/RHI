#pragma once
#include "RHI/Commands.hpp"
#include "RHI/Common.hpp"
#include "RHI/Memory.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Resource.hpp"
#include "RHI/ShaderResourceGroup.hpp"
#include "RHI/Swapchain.hpp"

namespace RHI
{

class IPhysicalDevice
{
public:
    enum class EVendor : uint8_t
    {
        Nvidia,
        Intel,
        AMD,
    };

    bool IsDiscrete() const
    {
        return m_isDiscrete;
    }

    bool IsIntegrated() const
    {
        return !m_isDiscrete;
    }

    EVendor GetVendor() const
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

    inline const IPhysicalDevice& GetPhysicalDevice() const
    {
        return *m_pPhysicalDevice;
    }

    virtual EResultCode WaitIdle() const = 0;

    virtual Expected<Unique<ISwapchain>> CreateSwapChain(
        const SwapchainDesc& desc) const = 0;

    virtual Expected<Unique<IShaderProgram>> CreateShaderProgram(
        const ShaderProgramDesc& desc) const = 0;

    virtual Expected<Unique<IShaderResourceGroupAllocator>>
    CreateShaderResourceGroupAllocator() const = 0;

    virtual Expected<Unique<IPipelineState>> CreateGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc) const = 0;

    virtual Expected<Unique<IFence>> CreateFence() const = 0;

    virtual Expected<Unique<ISampler>> CreateSampler(
        const SamplerDesc& desc) const = 0;

    virtual Expected<Unique<IImage>> CreateImage(
        const AllocationDesc& allocationDesc,
        const ImageDesc&      desc) const = 0;

    virtual Expected<Unique<IImageView>> CreateImageView(
        const IImage&        image,
        const ImageViewDesc& desc) const = 0;

    virtual Expected<Unique<IBuffer>> CreateBuffer(
        const AllocationDesc& allocationDesc,
        const BufferDesc&     desc) const = 0;

    virtual Expected<Unique<IBufferView>> CreateBufferView(
        const IBuffer&        buffer,
        const BufferViewDesc& desc) const = 0;

protected:
    IPhysicalDevice* m_pPhysicalDevice;
};

}  // namespace RHI
