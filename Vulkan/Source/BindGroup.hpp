#pragma once

#include <RHI/Result.hpp>
#include <RHI/BindGroup.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;
    struct IBindGroup;
    struct IBindGroupLayout;

    VkDescriptorType ConvertDescriptorType(BindingType bindingType);

    class BindGroupAllocator
    {
    public:
        BindGroupAllocator(IContext* context);
        ~BindGroupAllocator() = default;

        ResultCode Init();
        void Shutdown();

        ResultCode InitBindGroup(IBindGroup* bindGroup, IBindGroupLayout* bindGroupLayout, uint32_t bindlessElementsCount);
        void ShutdownBindGroup(IBindGroup* bindGroup);

    public:
        IContext* m_context;
        VkDescriptorPool m_descriptorPool;
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        BindGroupLayoutCreateInfo layoutInfo;
        VkDescriptorSetLayout handle;

        ResultCode Init(IContext* context, const BindGroupLayoutCreateInfo& createInfo);
        void Shutdown(IContext* context);
    };

    struct IBindGroup : BindGroup
    {
        VkDescriptorSet descriptorSet;
        Handle<BindGroupLayout> layout;

        ResultCode Init(IContext* context, Handle<BindGroupLayout> layout, uint32_t bindlessElementsCount);
        void Shutdown(IContext* context);

        void Write(IContext* context, TL::Span<const BindGroupUpdateInfo> bindings);
    };
} // namespace RHI::Vulkan