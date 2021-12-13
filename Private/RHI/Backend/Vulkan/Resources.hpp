#pragma once
#include "RHI/Resources.hpp"

#include "RHI/Backend/Vulkan/Common.hpp"
#include "RHI/Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

    class Semaphore : public DeviceObject<VkSemaphore>
    {
    public:
        Semaphore(Device& device)
            : DeviceObject(device)
        {
        }
        ~Semaphore();

        VkResult Init();
    };

    class RenderPass : public DeviceObject<VkRenderPass>
    {
    public:
        RenderPass(Device& device)
            : DeviceObject(device)
        {
        }
        ~RenderPass();

        size_t GetHash() const;
    };
    using RenderPassRef = Shared<RenderPass>;

    class Fence final
        : public IFence
        , public DeviceObject<VkFence>
    {
    public:
        Fence(Device& _device)
            : DeviceObject(_device)
        {
        }
        ~Fence();

        VkResult Init();

        virtual EResultCode Wait() const override;
        virtual EResultCode Reset() const override;
        virtual EResultCode GetStatus() const override;
    };

    class DeviceMemoryAllocation final
        : public IDeviceMemoryAllocation
        , public DeviceObject<VmaAllocation>
    {
    public:
        DeviceMemoryAllocation(Device& _device)
            : DeviceObject(_device)
        {
        }
        ~DeviceMemoryAllocation();

        VkResult Init(size_t size, size_t alignment, DeviceMemoryAllocationDesc _desc);
        VkResult InitForTexture(VkImage _image, DeviceMemoryAllocationDesc _desc);
        VkResult InitForBuffer(VkBuffer _buffer, DeviceMemoryAllocationDesc _desc);

        virtual size_t                  GetSize() const override;
        virtual Expected<DeviceAddress> Map(size_t offset, size_t range) const override;
        virtual EResultCode             Unmap() const override;

    private:
        VmaAllocationInfo m_allocationInfo;
    };

    // Textures. --------------------------------------------------------------------------------------
    class Texture final
        : public ITexture
        , public DeviceObject<VkImage>
    {

    public:
        Texture(Device& _device)
            : DeviceObject(_device)
        {
        }
        ~Texture();

        VkResult Init(const TextureDesc& _desc);
        VkResult InitForSwapChain(Extent3D _extent, EPixelFormat _format, VkImage _image);

        inline void SetAllocation(DeviceMemoryAllocation& allocation) { m_allocation = &allocation; }

        virtual Extent3D                 GetExtent() const override;
        virtual uint32_t                 GetMipLevelsCount() const override;
        virtual uint32_t                 GetArrayLayersCount() const override;
        virtual EPixelFormat             GetPixelFormat() const override;
		virtual TextureUsageFlags		 GetUsage() const override;
		virtual ESampleCount             GetSampleCount() const override;
        virtual IDeviceMemoryAllocation& GetAllocation() override;
    
    public:
        Extent3D     m_extent;
        uint32_t     m_mipLevelsCount;
        uint32_t     m_arrayLayersCount;
        EPixelFormat m_pixelFormat;
		ESampleCount m_sampleCount;
		TextureUsageFlags m_usage;
        DeviceMemoryAllocation* m_allocation;
    };

    class TextureView final
        : public ITextureView
        , public DeviceObject<VkImageView>
    {
    public:
        TextureView(Device& device)
            : DeviceObject(device)
        {
        }
        ~TextureView();

        VkResult Init(const TextureViewDesc& _desc);

        virtual ITexture&               GetUnderlyingTexture() const override;
        virtual TextureSubresourceRange GetSubresourceRange() const override;
    };

    // Buffers --------------------------------------------------------------------------------------

    class Buffer final
        : public IBuffer
        , public DeviceObject<VkBuffer>
    {
    public:
        Buffer(Device& _device)
            : DeviceObject(_device)
        {
        }
        ~Buffer();

        VkResult Init(const BufferDesc& _desc);

        inline void SetAllocation(DeviceMemoryAllocation& allocation) { m_allocation = &allocation; }

        virtual size_t                   GetSize() const override;
        virtual IDeviceMemoryAllocation& GetAllocation() override;

    private:
        size_t                  m_size;
        DeviceMemoryAllocation* m_allocation;
    };

    class SwapChain final
        : public ISwapChain
        , public DeviceObject<VkSwapchainKHR>
    {
        friend class Context;

    public:
        SwapChain(Device& _device)
            : DeviceObject(_device)
        {
        }
        ~SwapChain();

        VkResult Init(VkSurfaceKHR _surface, const SwapChainDesc& _desc);

        virtual EResultCode SwapBackBuffers() override;
        virtual ITexture*   GetBackBuffers() override;
        virtual uint32_t    GetBackBufferCount() const override;
        virtual uint32_t    GetCurrentBackBufferIndex() const override;

    private:
        std::vector<VkSemaphore> m_ImageAvailableSemaphores;

        std::vector<Texture> m_backBuffers;

        uint32_t m_currentFrameIndex;
        uint32_t m_currentImageIndex;
    };

    // Sampler --------------------------------------------------------------------------------------

    class Sampler final
        : public ISampler
        , public DeviceObject<VkSampler>
    {
    public:
        Sampler(Device& _device)
            : DeviceObject(_device)
        {
        }
        ~Sampler();

        VkResult Init(const SamplerDesc& _desc);
    };

    // RenderTarget --------------------------------------------------------------------------------------

    class RenderTarget final
        : public IRenderTarget
        , public DeviceObject<VkFramebuffer>
    {
    public:
        RenderTarget(Device& device)
            : DeviceObject(device){};
        ~RenderTarget();

        VkResult Init(VkRenderPass renderPass, const RenderTargetDesc& _desc);

        virtual std::vector<EPixelFormat> GetColorAttachmentFormats() const override;
        virtual EPixelFormat              GetDepthStencilAttachmentFormat() const override;
        virtual Extent2D                  GetExtent() const override;
        virtual uint32_t                  GetCount() const override;
        virtual ITextureView**            GetAttachments() override;

    private:
        std::vector<ITextureView*> m_pAttachments;
        Extent2D                   m_extent;
    };

} // namespace Vulkan
} // namespace RHI
