#pragma once

#include <RHI/BindGroup.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;
    struct IBindGroup;
    struct IBindGroupLayout;

    static constexpr uint32_t MaxShaderBindingsCount = 32;

    VkDescriptorType ConvertDescriptorType(BindingType bindingType);

    class BindGroupAllocator
    {
    public:
        BindGroupAllocator();
        ~BindGroupAllocator();

        ResultCode Init(IDevice* device);
        void       Shutdown();

        ResultCode InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout);
        void       ShutdownBindGroup(IBindGroup* bindGroup);

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

        ResultCode Init(IDevice* device, Handle<BindGroupLayout> layout);
        void       Shutdown(IDevice* device);

        void Write(IDevice* device, const BindGroupUpdateInfo& updateInfo);
    };
} // namespace RHI::Vulkan