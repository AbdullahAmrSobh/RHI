#pragma once

#include "RHI/Shader.hpp"
#include "RHI/Handle.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Pipeline.hpp"

namespace RHI
{
    inline static constexpr uint32_t c_MaxBindGroupElementsCount = 32u;

    struct Buffer;
    struct BufferView;

    struct Image;
    struct ImageView;

    struct Sampler;

    RHI_DECLARE_OPAQUE_RESOURCE(BindGroupLayout);
    RHI_DECLARE_OPAQUE_RESOURCE(BindGroup);

    enum class BindingType
    {
        None,
        Sampler,
        SampledImage,
        StorageImage,
        UniformBuffer,
        StorageBuffer,
        DynamicUniformBuffer,
        DynamicStorageBuffer,
        BufferView,
        StorageBufferView,
        Count,
    };

    struct ShaderBinding
    {
        inline static constexpr uint32_t VariableArraySize = UINT32_MAX;

        BindingType                      type;
        Access                           access;
        uint32_t                         arrayCount;
        TL::Flags<ShaderStage>           stages;
    };

    struct BindGroupLayoutCreateInfo
    {
        ShaderBinding bindings[c_MaxBindGroupElementsCount];
    };

    struct BindGroupUpdateInfo
    {
        enum class Type
        {
            Image,
            Buffer,
            DynamicBuffer,
            Sampler,
        };

        struct DynamicBufferBinding
        {
            Handle<Buffer> buffer;
            size_t         offset, range;
        };

        union ResourceData
        {
            ResourceData() {}

            ~ResourceData() {}

            TL::Span<const Handle<ImageView>>    images;
            TL::Span<const Handle<Buffer>>       buffers;
            TL::Span<const DynamicBufferBinding> dynamicBuffers;
            TL::Span<const Handle<Sampler>>      samplers;
        };

        uint32_t     binding;
        uint32_t     dstArrayElement;
        Type         type;
        ResourceData data;

        BindGroupUpdateInfo(uint32_t binding, uint32_t dstArrayElement, TL::Span<const Handle<ImageView>> images)
            : binding(binding)
            , dstArrayElement(dstArrayElement)
            , type(Type::Image)
        {
            data.images = images;
        }

        BindGroupUpdateInfo(uint32_t binding, uint32_t dstArrayElement, TL::Span<const Handle<Buffer>> buffers)
            : binding(binding)
            , dstArrayElement(dstArrayElement)
            , type(Type::Buffer)
        {
            data.buffers = buffers;
        }

        BindGroupUpdateInfo(uint32_t binding, uint32_t dstArrayElement, TL::Span<const DynamicBufferBinding> dynamicBuffers)
            : binding(binding)
            , dstArrayElement(dstArrayElement)
            , type(Type::DynamicBuffer)
        {
            data.dynamicBuffers = dynamicBuffers;
        }

        BindGroupUpdateInfo(uint32_t binding, uint32_t dstArrayElement, TL::Span<const Handle<Sampler>> samplers)
            : binding(binding)
            , dstArrayElement(dstArrayElement)
            , type(Type::Sampler)
        {
            data.samplers = samplers;
        }
    };

} // namespace RHI