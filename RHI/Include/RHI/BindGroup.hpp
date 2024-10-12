#pragma once

#include "RHI/Handle.hpp"
#include "RHI/Pipeline.hpp"
#include "RHI/PipelineAccess.hpp"
#include "RHI/Image.hpp"
#include "RHI/Sampler.hpp"
#include "RHI/Buffer.hpp"

namespace RHI
{
    /// @brief Defines the maximum size for bindless arrays.
    inline static constexpr uint32_t BindlessArraySize = UINT32_MAX;

    struct Buffer;
    struct BufferView;
    struct Image;
    struct ImageView;
    struct Sampler;

    RHI_DECLARE_OPAQUE_RESOURCE(BindGroupLayout);
    RHI_DECLARE_OPAQUE_RESOURCE(BindGroup);

    /// @brief Specifies the type of resource bound to a shader.
    enum class BindingType
    {
        None,                 ///< No binding.
        Sampler,              ///< Sampler resource.
        SampledImage,         ///< Sampled image (read-only texture).
        StorageImage,         ///< Storage image (read/write texture).
        UniformBuffer,        ///< Uniform buffer (constant data).
        StorageBuffer,        ///< Storage buffer (read/write data).
        DynamicUniformBuffer, ///< Dynamic uniform buffer.
        DynamicStorageBuffer, ///< Dynamic storage buffer.
        BufferView,           ///< Buffer view.
        StorageBufferView,    ///< Storage buffer view.
        Count,                ///< Number of binding types.
    };

    /// @brief Describes a shader binding, including its type, access, and stages.
    struct ShaderBinding
    {
        BindingType            type;       ///< Type of the binding.
        Access                 access;     ///< Access type (read/write) for the resource.
        uint32_t               arrayCount; ///< Number of elements in the array for this binding.
        TL::Flags<ShaderStage> stages;     ///< Shader stages where this binding is accessible.
    };

    /// @brief Information required to create a bind group layout.
    struct BindGroupLayoutCreateInfo
    {
        const char*                   name;     ///< Name of the bind group layout.
        TL::Span<const ShaderBinding> bindings; ///< Span of shader bindings for this layout.
    };

    /// @brief Information used to update images in a bind group.
    struct BindGroupImagesUpdateInfo
    {
        uint32_t                          dstBinding;      ///< Target binding index.
        uint32_t                          dstArrayElement; ///< Target array element for binding.
        TL::Span<const Handle<ImageView>> images;          ///< Span of image views to bind.
    };

    /// @brief Information used to update buffers in a bind group.
    struct BindGroupBuffersUpdateInfo
    {
        uint32_t                        dstBinding;      ///< Target binding index.
        uint32_t                        dstArrayElement; ///< Target array element for binding.
        TL::Span<const Handle<Buffer>>  buffer;          ///< Span of buffer handles to bind.
        TL::Span<const BufferSubregion> subregions;      ///< Span of buffer subregions to bind.
    };

    /// @brief Information used to update samplers in a bind group.
    struct BindGroupSamplersUpdateInfo
    {
        uint32_t                        dstBinding;      ///< Target binding index.
        uint32_t                        dstArrayElement; ///< Target array element for binding.
        TL::Span<const Handle<Sampler>> samplers;        ///< Span of sampler handles to bind.
    };

    /// @brief General update information for a bind group, encompassing images, buffers, and samplers.
    struct BindGroupUpdateInfo
    {
        TL::Span<const BindGroupImagesUpdateInfo>   images;   ///< Image updates for the bind group.
        TL::Span<const BindGroupBuffersUpdateInfo>  buffers;  ///< Buffer updates for the bind group.
        TL::Span<const BindGroupSamplersUpdateInfo> samplers; ///< Sampler updates for the bind group.
    };
} // namespace RHI
