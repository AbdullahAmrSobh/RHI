#pragma once

#include <RHI/Device.hpp>
#include <RHI/Resources.hpp>
#include <RHI/Result.hpp>

#include <vk_mem_alloc.h>

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

    static constexpr uint32_t MaxShaderBindingsCount = 32;

    class BindGroupAllocator
    {
    public:
        BindGroupAllocator();
        ~BindGroupAllocator();

        ResultCode Init(IDevice* device);
        void       Shutdown();

        ResultCode InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout);
        void       ShutdownBindGroup(IBindGroup* bindGroup);

        void Reset();

    public:
        IDevice*         m_device;
        VkDescriptorPool m_descriptorPool;
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        VkDescriptorSetLayout handle;
        ShaderBinding         shaderBindings[MaxShaderBindingsCount];
        uint32_t              bindlessCount;

        ResultCode Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBindGroup : BindGroup
    {
        VkDescriptorSet descriptorSet;
        ShaderBinding   shaderBindings[MaxShaderBindingsCount];
        uint32_t        bindlessCount;

        ResultCode Init(IDevice* device, const BindGroupCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void Write(IDevice* device, const BindGroupUpdateInfo& updateInfo);
    };

    class IShaderModule final : public ShaderModule
    {
    public:
        IShaderModule();
        ~IShaderModule();

        ResultCode Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void       Shutdown();

    public:
        IDevice*       m_device;
        VkShaderModule m_shaderModule;
    };

    struct IPipelineLayout : PipelineLayout
    {
        VkPipelineLayout handle;

        ResultCode Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        VkPipeline       handle;
        VkPipelineLayout layout;

        ResultCode Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IComputePipeline : ComputePipeline
    {
        VkPipeline       handle;
        VkPipelineLayout layout;

        ResultCode Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBuffer : Buffer
    {
        VkBuffer      handle;
        VmaAllocation allocation;

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
} // namespace RHI::Vulkan