#pragma once

#include <TL/Containers/Vector.hpp>

#ifdef RHI_PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
#else
    #error "Current platfrom is not supported yet"
#endif

#include <RHI/RHI.h>

#include <TL/Containers/InlineVector.hpp>
#include <TL/Utils.hpp>

#include <vk_mem_alloc.h>

namespace RHI::Vulkan
{
    struct IDevice;
    struct IBindGroup;
    struct IBindGroupLayout;

    inline static VkExtent2D ConvertExtent2D(ImageSize2D size)
    {
        return {size.width, size.height};
    }

    inline static VkExtent3D ConvertExtent3D(ImageSize3D size)
    {
        return {size.width, size.height, size.depth};
    }

    inline static VkOffset2D ConvertOffset2D(ImageOffset2D offset)
    {
        return {offset.x, offset.y};
    }

    inline static VkOffset3D ConvertOffset3D(ImageOffset3D offset)
    {
        return {offset.x, offset.y, offset.z};
    }

    VkImageAspectFlags      ConvertImageAspect(TL::Flags<ImageAspect> imageAspect, Format format);
    VkImageSubresourceRange ConvertSubresourceRange(const ImageSubresourceRange& subresource, Format format);

    inline static VkDescriptorType ConvertDescriptorType(BindingType bindingType)
    {
        switch (bindingType)
        {
        case BindingType::None: break;
        case BindingType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
        case BindingType::SampledImage: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case BindingType::StorageImage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case BindingType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case BindingType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case BindingType::UniformBufferDynamic: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case BindingType::StorageBufferDynamic: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case BindingType::UniformTexelBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case BindingType::StorageTexelBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case BindingType::InputAttachment: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        case BindingType::AccelerationStructure: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        case BindingType::Count: break;
        }
        TL_UNREACHABLE();
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

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

    struct IFence : Fence
    {
        IFence(TL::StringView name = {})
            : Fence(name)
        {
        }

        VkSemaphore          semaphore;
        std::atomic_uint64_t value;

        ResultCode Init(IDevice* device, const FenceCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
        bool       waitValue(IDevice* device, uint64_t value);
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        IBindGroupLayout(TL::StringView name = {})
            : BindGroupLayout(name)
        {
        }

        VkDescriptorSetLayout                 handle = VK_NULL_HANDLE;
        TL::InlineVector<ShaderBinding, 64> shaderBindings;
        bool                                  hasBindless = false;

        ResultCode Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        ShaderBinding GetBinding(uint32_t binding) const { return shaderBindings[binding]; }
    };

    struct IBindGroup : BindGroup
    {
        IBindGroup(TL::StringView name = {})
            : BindGroup(name)
        {
        }

        VkDescriptorSet   descriptorSet;
        IBindGroupLayout* bindGroupLayout;

        ResultCode Init(IDevice* device, const BindGroupCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void Update(IDevice* device, const BindGroupUpdateInfo& updateInfo);
    };

    struct IShaderModule : ShaderModule
    {
        IShaderModule(TL::StringView name = {})
            : ShaderModule(name)
        {
        }

        VkShaderModule m_shaderModule;

        ResultCode Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IPipelineLayout : PipelineLayout
    {
        IPipelineLayout(TL::StringView name = {})
            : PipelineLayout(name)
        {
        }

        VkPipelineLayout   handle;
        IBindGroupLayout*  bindGroupLayouts[4];
        VkShaderStageFlags pushConstantStages = 0;

        ResultCode Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        IGraphicsPipeline(TL::StringView name = {})
            : GraphicsPipeline(name)
        {
        }

        VkPipeline       handle;
        IPipelineLayout* layout;

        ResultCode Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IComputePipeline : ComputePipeline
    {
        IComputePipeline(TL::StringView name = {})
            : ComputePipeline(name)
        {
        }

        VkPipeline       handle;
        IPipelineLayout* layout;

        ResultCode Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IRayTracingPipeline : RayTracingPipeline
    {
        IRayTracingPipeline(TL::StringView name = {})
            : RayTracingPipeline(name)
        {
        }

        VkPipeline       handle;
        IPipelineLayout* layout;

        ResultCode Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void GetShaderBindingTableEntry(IDevice* device, uint32_t group, size_t size, void* dstHandle);
    };

    struct IQueryPool : QueryPool
    {
        IQueryPool(TL::StringView name = {})
            : QueryPool(name)
        {
        }

        VkQueryPool handle;

        ResultCode Init(IDevice* device, const QueryPoolCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBuffer : Buffer
    {
        IBuffer(TL::StringView name = {})
            : Buffer(name)
        {
        }

        VkBuffer        handle;
        VmaAllocation   allocation;
        VkDeviceAddress address;

        ResultCode Init(IDevice* device, const BufferCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        DeviceMemoryPtr Map(IDevice* device);
        void            Unmap(IDevice* device);
    };

    struct IImage : Image
    {
        IImage(TL::StringView name = {})
            : Image(name)
        {
        }

        VkImage       handle;
        VkImageView   viewHandle;
        VmaAllocation allocation;

        // TODO: the following should be removed
        ImageSize3D           size;
        Format                format;
        ImageSubresourceRange subresources;

        ResultCode Init(IDevice* device, const ImageCreateInfo& createInfo);
        ResultCode Init(IDevice* device, const ImageViewCreateInfo& createInfo);
        ResultCode Init(IDevice* device, VkImage image, const VkSwapchainCreateInfoKHR& swapchainCreateInfo);
        void       Shutdown(IDevice* device);
    };

    struct ISampler : Sampler
    {
        ISampler(TL::StringView name = {})
            : Sampler(name)
        {
        }

        VkSampler handle;

        ResultCode Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IAccelerationStructure : AccelerationStructure
    {
        IAccelerationStructure(TL::StringView name = {})
            : AccelerationStructure(name)
        {
        }

        VkAccelerationStructureKHR handle = VK_NULL_HANDLE;

        VkDeviceAddress address = 0;

        AccelerationStructureSizesInfo sizes = {};

        // Build flags chosen at creation; the size query and the actual build must use the
        // same flags or the build will require more memory than the AS/scratch was sized for.
        VkBuildAccelerationStructureFlagsKHR buildFlags = {};

        // TODO: instead of having a buffer per AS, we should suballocate from a buffer pool
        VkBuffer          buffer         = VK_NULL_HANDLE;
        VmaAllocation     allocation     = VK_NULL_HANDLE;
        VmaAllocationInfo allocationInfo = {};
        VkDeviceAddress   bufferAddress  = 0;

        ResultCode Init(IDevice* device, const AccelerationStructureCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IMicromap : Micromap
    {
        IMicromap(TL::StringView name = {})
            : Micromap(name)
        {
        }

        VkMicromapEXT handle;
        VmaAllocation allocation;

        ResultCode Init(IDevice* device, const MicromapCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct ISwapchain : Swapchain
    {
        ISwapchain(TL::StringView name = {})
            : Swapchain(name)
        {
        }

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        // Interface
        uint32_t               GetImagesCount() const;
        SwapchainAcquireResult AcquireSwapchainImage(IDevice* device);
        SurfaceCapabilities    GetSurfaceCapabilities(IDevice* device) const;
        ResultCode             ResizeSwapchain(IDevice* device, const ImageSize2D& size);
        ResultCode             ConfigureSwapchain(IDevice* device, const SwapchainConfigureInfo& configInfo);
        VkResult               AcquireNextImage(IDevice* device);
        VkResult               Present(IDevice* device, TL::Span<Fence* const> fences);

        constexpr static auto MaxImageCount = 4;

        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        VkSurfaceKHR   m_surface   = VK_NULL_HANDLE;

        uint32_t               m_acquireSemaphoreIndex           = 0;
        VkSemaphore            m_acquireSemaphore[MaxImageCount] = {};
        uint32_t               m_currentAcquireIndex             = 0;
        IFence                 m_acquireFences[MaxImageCount]    = {};
        VkSemaphore            m_presentSemaphore[MaxImageCount] = {};
        uint32_t               m_imageIndex                      = {};
        bool                   m_imageAcquired                   = false;
        VkImage                m_images[MaxImageCount]           = {};
        VkImageView            m_imageViews[MaxImageCount]       = {};
        SwapchainConfigureInfo m_configuration                   = {};
        uint32_t               m_imageCount                      = 0;
        IImage*                m_imageHandle                     = nullptr;
    };

    VkResult CreateSurface(IDevice& device, const SwapchainCreateInfo& createInfo, VkSurfaceKHR& outSurface);
} // namespace RHI::Vulkan
