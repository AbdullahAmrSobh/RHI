#pragma once
#include "RHI/Resource.hpp"

#include <vulkan/vulkan.h>

namespace RHI
{
namespace Vulkan
{
    class Device;

    template <typename T>
    class Resource
    {
    public:
        T GetHandle() const
        {
            return m_handle;
        }

    private:
        Device* m_pDevice;
        T       m_handle;
    };

    class ShaderModule final
        : public IShaderProgram
        , public Resource<VkShaderModule>
    {
    public:
        ShaderModule(Device& device);
        ~ShaderModule();

        VkResult Init(const ShaderModuleDesc& desc);
    };

    class Fence final
        : public IFence
        , public Resource<VkFence>
    {
    public:
        Fence(Device& device);
        ~Fence();

        VkResult Init(const FenceDesc& desc);
    };

    class Image final
        : public IImage
        , public Resource<VkImage>
    {
    public:
        Image(Device& device);
        ~Image();

        VkResult Init(const ImageDesc& desc);
    };

    class ImageView final
        : public IImageView
        , public Resource<VkImageView>
    {
    public:
        ImageView(Device& device);
        ~ImageView();

        VkResult Init(const ImageViewDesc& desc);
    };

    class Buffer final
        : public IBuffer
        , public Resource<VkBuffer>
    {
    public:
        Buffer(Device& device);
        ~Buffer();

        VkResult Init(const BufferDesc& desc);
    };

    class BufferView final
        : public IBufferView
        , public Resource<VkBufferView>
    {
    public:
        BufferView(Device& device);
        ~BufferView();

        VkResult Init(const BufferViewDesc& desc);
    };

    class Sampler final
        : public ISampler
        , public Resource<VkSampler>
    {
    public:
        Sampler(Device& device);
        ~Sampler();

        VkResult Init(const SamplerDesc& desc);
    };

} // namespace Vulkan
} // namespace RHI