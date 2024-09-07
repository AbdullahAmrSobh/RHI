#pragma once

#include <RHI/Result.hpp>
#include <RHI/BindGroup.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;
    struct IBindGroup;
    struct IBindGroupLayout;

    static constexpr uint32_t MaxShaderBindingsCount = 32;

    VkDescriptorType ConvertDescriptorType(BindingType bindingType);

    class BindGroupAllocator
    {
    public:
        BindGroupAllocator(IContext* context);
        ~BindGroupAllocator() = default;

        ResultCode Init();
        void Shutdown();

        ResultCode InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout);
        void ShutdownBindGroup(IBindGroup* bindGroup);

    public:
        IContext* m_context;
        VkDescriptorPool m_descriptorPool;
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        VkDescriptorSetLayout handle;
        ShaderBinding shaderBindings[MaxShaderBindingsCount];
        uint32_t bindlessCount;

        ResultCode Init(IContext* context, const BindGroupLayoutCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    struct IBindGroup : BindGroup
    {
        VkDescriptorSet descriptorSet;
        ShaderBinding shaderBindings[MaxShaderBindingsCount];
        uint32_t bindlessCount;

        ResultCode Init(IContext* context, Handle<BindGroupLayout> layout);
        void Shutdown(IContext* context);

        void Write(IContext* context, const BindGroupUpdateInfo& updateInfo);
    };
} // namespace RHI::Vulkan