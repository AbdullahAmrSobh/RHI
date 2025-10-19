#pragma once

#include <RHI/Device.hpp>
#include <RHI/Resources.hpp>
#include <RHI/Result.hpp>

#include <vk_mem_alloc.h>

#include <TL/Context.hpp>
#include <TL/Stacktrace.hpp>
#include <TL/Utils.hpp>

namespace RHI::Vulkan
{
    class IDevice;
    struct IBindGroup;
    struct IBindGroupLayout;

    VkBufferUsageFlags      ConvertBufferUsageFlags(TL::Flags<BufferUsage> bufferUsageFlags);
    VkImageUsageFlags       ConvertImageUsageFlags(TL::Flags<ImageUsage> imageUsageFlags);
    VkImageType             ConvertImageType(ImageType imageType);
    VkImageViewType         ConvertImageViewType(ImageViewType imageType);
    VkImageAspectFlags      ConvertImageAspect(TL::Flags<ImageAspect> imageAspect, Format format);
    VkComponentSwizzle      ConvertComponentSwizzle(ComponentSwizzle componentSwizzle);
    VkImageSubresourceRange ConvertSubresourceRange(const ImageSubresourceRange& subresource, Format format);
    VkComponentMapping      ConvertComponentMapping(ComponentMapping componentMapping);
    VkExtent2D              ConvertExtent2D(ImageSize2D size);
    VkExtent3D              ConvertExtent3D(ImageSize3D size);
    VkExtent2D              ConvertExtent2D(ImageSize3D size);
    VkOffset2D              ConvertOffset2D(ImageOffset2D offset);
    VkOffset3D              ConvertOffset3D(ImageOffset3D offset);
    VkFilter                ConvertFilter(SamplerFilter samplerFilter);
    VkSamplerAddressMode    ConvertSamplerAddressMode(SamplerAddressMode addressMode);
    VkCompareOp             ConvertCompareOp(CompareOperator compareOperator);
    VkShaderStageFlags      ConvertShaderStage(TL::Flags<ShaderStage> shaderStageFlags);
    VkDescriptorType        ConvertDescriptorType(BindingType bindingType);

    class DescriptorSetWriter
    {
    public:
        DescriptorSetWriter(IDevice* device, VkDescriptorSet descriptorSet, IBindGroupLayout* layout, TL::IAllocator& allocator);
        VkWriteDescriptorSet BindImages(uint32_t dstBinding, uint32_t dstArray, TL::Span<Image* const> images);
        VkWriteDescriptorSet BindSamplers(uint32_t dstBinding, uint32_t dstArray, TL::Span<Sampler* const> samplers);
        VkWriteDescriptorSet BindBuffers(uint32_t dstBinding, uint32_t dstArray, TL::Span<const BufferBindingInfo> buffers);

        TL::Span<const VkWriteDescriptorSet> GetWrites() const { return m_writes; }

    private:
        IDevice*                                       m_device;
        TL::IAllocator*                                m_allocator;
        BindGroupLayout*                               m_bindGroupLayout;
        VkDescriptorSet                                m_descriptorSet;
        TL::Vector<TL::Vector<VkDescriptorImageInfo>>  m_images;
        TL::Vector<TL::Vector<VkDescriptorImageInfo>>  m_sampler;
        TL::Vector<TL::Vector<VkDescriptorBufferInfo>> m_buffers;
        TL::Vector<TL::Vector<VkBufferView>>           m_bufferViews;
        TL::Vector<VkWriteDescriptorSet>               m_writes;
    };

    class BindGroupAllocator
    {
    public:
        BindGroupAllocator();
        ~BindGroupAllocator();

        VkResult Init(IDevice* device);
        void     Shutdown();

        ResultCode InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout, uint32_t bindlessResourcesCount);
        void       ShutdownBindGroup(IBindGroup* bindGroup);

        void Reset();

    public:
        IDevice*         m_device;
        VkDescriptorPool m_descriptorPool;
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        VkDescriptorSetLayout     handle;
        TL::Vector<ShaderBinding> shaderBindings;
        bool                      hasBindless;

        ResultCode Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        ShaderBinding GetBinding(uint32_t binding) { return shaderBindings[binding]; }
    };

    struct IBindGroup : BindGroup
    {
        VkDescriptorSet   descriptorSet;
        IBindGroupLayout* bindGroupLayout;

        ResultCode Init(IDevice* device, const BindGroupCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void Update(IDevice* device, const BindGroupUpdateInfo& updateInfo);
    };

    class IShaderModule final : public ShaderModule
    {
    public:
        IShaderModule();
        ~IShaderModule();

        ResultCode Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

    public:
        IDevice*       m_device;
        VkShaderModule m_shaderModule;
    };

    struct IPipelineLayout : PipelineLayout
    {
        VkPipelineLayout handle;
        BindGroupLayout* bindGroupLayouts[4];

        ResultCode Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        VkPipeline       handle;
        IPipelineLayout* layout;

        ResultCode Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IComputePipeline : ComputePipeline
    {
        VkPipeline       handle;
        IPipelineLayout* layout;

        ResultCode Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IRayTracingPipeline : RayTracingPipeline
    {
        VkPipeline       handle;
        IPipelineLayout* layout;

        ResultCode Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBuffer : Buffer
    {
        VkBuffer      handle;
        VmaAllocation allocation;
        bool          mapped;

        ResultCode Init(IDevice* device, const BufferCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        VkMemoryRequirements GetMemoryRequirements(IDevice* device) const;

        DeviceMemoryPtr Map(IDevice* device);
        void            Unmap(IDevice* device);
    };

    struct IImage : Image
    {
        VkImage       handle;
        VkImageView   viewHandle;
        VmaAllocation allocation;

        // TODO: the following should be removed
        ImageSize3D           size;
        Format                format;
        ImageSubresourceRange subresources;

        ResultCode Init(IDevice* device, const ImageCreateInfo& createInfo);
        ResultCode Init(IDevice* device, VkImage image, const VkSwapchainCreateInfoKHR& swapchainCreateInfo);
        void       Shutdown(IDevice* device);

        VkMemoryRequirements GetMemoryRequirements(IDevice* device) const;

        // Selects specifc aspect from the available image aspects
        VkImageAspectFlags SelectImageAspect(ImageAspect aspect);
    };

    struct ISampler : Sampler
    {
        VkSampler handle;

        ResultCode Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    class ISwapchain final : public Swapchain
    {
    public:
        constexpr static auto MaxImageCount = 4;

        ISwapchain();
        ~ISwapchain();

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        static VkResult CreateSurface(IDevice& device, const SwapchainCreateInfo& createInfo, VkSurfaceKHR& outSurface);

        // Interface
        uint32_t            GetImagesCount() const override;
        Image*              GetImage() const override;
        SurfaceCapabilities GetSurfaceCapabilities() const override;
        ResultCode          Resize(const ImageSize2D& size) override;
        ResultCode          Configure(const SwapchainConfigureInfo& configInfo) override;

        VkResult       AcquireNextImage(VkSemaphore& acquireImageSemaphore);
        uint32_t       GetImageIndex() const;
        VkSwapchainKHR GetHandle() const;

    private:
        IDevice*               m_device                          = nullptr;
        VkSwapchainKHR         m_swapchain                       = VK_NULL_HANDLE;
        VkSurfaceKHR           m_surface                         = VK_NULL_HANDLE;
        uint32_t               m_acquireSemaphoreIndex           = 0;
        VkSemaphore            m_acquireSemaphore[MaxImageCount] = {};
        uint32_t               m_imageIndex                      = {};
        VkImage                m_images[MaxImageCount]           = {};
        VkImageView            m_imageViews[MaxImageCount]       = {};
        TL::String             m_name                            = {};
        SwapchainConfigureInfo m_configuration                   = {};
        uint32_t               m_imageCount                      = 0;
        struct IImage*         m_imageHandle                     = nullptr;
    };
} // namespace RHI::Vulkan