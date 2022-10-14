#pragma once
#include "RHI/Resource.hpp"
#include "Backend/Vulkan/Common.hpp"

namespace RHI
{
namespace Vulkan
{
    class Device;

    template <typename T>
    class DeviceObject
    {
    public:
        DeviceObject(const Device* pDevice, T handle = VK_NULL_HANDLE)
            : m_pDevice(pDevice)
            , m_handle(handle)
        {
        }
        
        inline T GetHandle() const
        {
            return m_handle;
        }
    
    protected:
        const Device* m_pDevice;
        T       m_handle;
    };

    template <typename T>
    class Resource : public DeviceObject<T>
    {
    public:
        Resource(const Device* pDevice, T handle = VK_NULL_HANDLE, VmaAllocation allocation = VK_NULL_HANDLE)
            : DeviceObject<T>(pDevice, handle)
            , m_allocation(allocation)
        {
        }

        inline VmaAllocation GetAllocation() const
        {
            return m_allocation;
        }

        inline const VmaAllocationInfo& GetAllocationInfo() const
        {
            return m_allocationInfo;
        }

    protected:
        VmaAllocation     m_allocation = VK_NULL_HANDLE;
        VmaAllocationInfo m_allocationInfo;
    };

    class ShaderModule final
        : public IShaderProgram
        , public DeviceObject<VkShaderModule>
    {
    public:
        ShaderModule(Device& device);
        ~ShaderModule();

        VkResult Init(const ShaderProgramDesc& desc);
    };

    class Fence final
        : public IFence
        , public DeviceObject<VkFence>
    {
    public:
        Fence(Device& device)
            : DeviceObject(&device)
        {
        }

        ~Fence();

        VkResult Init();

        virtual EResultCode Wait() const override;
        virtual EResultCode Reset() const override;
        virtual EResultCode GetStatus() const override;
    };

    class Image final
        : public IImage
        , public Resource<VkImage>
    {
    public:
        Image(Device& device, VkImage imageHandle = VK_NULL_HANDLE)
            : Resource(&device, imageHandle)
        {
        }
        ~Image();

        VkResult Init(const AllocationDesc& allocationDesc, const ImageDesc& desc);
    };

    class ImageView final
        : public IImageView
        , public DeviceObject<VkImageView>
    {
    public:
        ImageView(Device& device)
            : DeviceObject(&device)
        {
        }
        ~ImageView();

        VkResult Init(const Image& image, const ImageViewDesc& desc);
    };

    class Buffer final
        : public IBuffer
        , public Resource<VkBuffer>
    {
    public:
        Buffer(Device& device)
            : Resource(&device)
        {
        }
        ~Buffer();

        VkResult Init(const AllocationDesc& allocationDesc, const BufferDesc& desc);
    };

    class BufferView final
        : public IBufferView
        , public DeviceObject<VkBufferView>
    {
    public:
        BufferView(Device& device)
            : DeviceObject(&device)
        {
        }
        ~BufferView();

        VkResult Init(const Buffer& buffer, const BufferViewDesc& desc);
    };

    class Sampler final
        : public ISampler
        , public DeviceObject<VkSampler>
    {
    public:
        Sampler(Device& device)
            : DeviceObject(&device)
        {
        }
        ~Sampler();

        VkResult Init(const SamplerDesc& desc);
    };

    class Semaphore final : public DeviceObject<VkSemaphore>
    {
    public:
        Semaphore(Device& device)
            : DeviceObject(&device)
        {
        }
        ~Semaphore();

        VkResult Init(bool bin);
    };

} // namespace Vulkan
} // namespace RHI