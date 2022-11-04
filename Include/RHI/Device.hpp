#pragma once
#include "RHI/Commands.hpp"
#include "RHI/Common.hpp"
#include "RHI/FrameGraph.hpp"
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

    inline EVendor GetVendor() const
    {
        return m_vendor;
    }

private:
    bool    m_isDiscrete;
    EVendor m_vendor;
};

class ITransferQueue
{
public:
    virtual EResultCode Enqueue(const CopyCommand& copyCommand, const IFence& signalFence)                     = 0;
    virtual EResultCode Enqueue(const std::vector<const CopyCommand&> copyCommands, const IFence& signalFence) = 0;
};

class IDevice
{
public:
    virtual ~IDevice() = default;

    inline const IPhysicalDevice& GetPhysicalDevice() const
    {
        return *m_pPhysicalDevice;
    }

    inline const ITransferQueue& GetTransferQueue() const
    {
        return *m_transferQueue;
    }

    virtual EResultCode WaitIdle() const = 0;
    
    virtual Expected<Unique<ISwapchain>> CreateSwapChain(const SwapchainDesc& desc) = 0;

    virtual Expected<Unique<IShaderProgram>> CreateShaderProgram(const ShaderProgramDesc& desc) = 0;

    virtual Expected<Unique<IShaderResourceGroupAllocator>> CreateShaderResourceGroupAllocator() = 0;

    virtual Expected<Unique<IPipelineState>> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) = 0;

    virtual Expected<Unique<IFence>> CreateFence() = 0;

    virtual Expected<Unique<ISampler>> CreateSampler(const SamplerDesc& desc) = 0;

    virtual Expected<Unique<IImage>> CreateImage(const AllocationDesc& allocationDesc, const ImageDesc& desc) = 0;
    
    virtual Expected<Unique<IImageView>> CreateImageView(const IImage& image, const ImageViewDesc& desc) = 0;

    virtual Expected<Unique<IBuffer>> CreateBuffer(const AllocationDesc& allocationDesc, const BufferDesc& desc) = 0;

    virtual Expected<Unique<IBufferView>> CreateBufferView(const IBuffer& buffer, const BufferViewDesc& desc) = 0;

    virtual Expected<Unique<IFrameGraph>> CreateFrameGraph() = 0;

protected:
    Unique<ITransferQueue> m_transferQueue;
    IPhysicalDevice*       m_pPhysicalDevice;
};
using DevicePtr = Unique<IDevice>;

} // namespace RHI
