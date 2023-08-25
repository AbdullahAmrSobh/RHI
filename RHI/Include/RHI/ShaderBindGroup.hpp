#pragma once
#include <array>
#include <vector>

#include "RHI/Handle.hpp"
#include "RHI/Object.hpp"
#include "RHI/Span.hpp"

namespace RHI
{

/// Resources forward declaration.
class Context;
class ImageView;
class BufferView;
class Sampler;

/// @brief The type of the shader resource to be bound.
enum class ShaderBindingType
{
    None,
    Sampler,
    Image,
    Buffer,
};

/// @brief How the resource will be accessed in the shader.
enum class ShaderBindingAccess
{
    OnlyRead,
    ReadWrite,
};

/// @brief Specifies a single shader resource binding.
struct ShaderBinding
{
    ShaderBindingType   type;
    ShaderBindingAccess access;
    uint32_t            arrayCount;
};

/// @brief A shader bind group layout is an list of shader bindings.
struct ShaderBindGroupLayout
{
    TL::Span<ShaderBinding> bindings;
};

/// @brief An object that groups shader resources that are bound together.
class ShaderBindGroup
{
public:
    virtual ~ShaderBindGroup() = default;

    /// @brief Binds an image resource to the provided binding index and offset array index.
    /// NOTE: offset + images count should not exceed the count of the resources decalred in the layout or the shader.
    /// @param index index of the resource binding decelration in the shader.
    /// @param images list of handles of an actual resources to bind.
    /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
    virtual void BindImages(uint32_t index, TL::Span<Handle<ImageView>> images, uint32_t arrayOffset = 0) = 0;

    /// @brief Binds an image resource to the provided binding index and offset array index.
    /// NOTE: offset + buffers count should not exceed the count of the resources decalred in the layout or the shader.
    /// @param index index of the resource binding decelration in the shader.
    /// @param buffers list of handles of an actual resources to bind.
    /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
    virtual void BindBuffers(uint32_t index, TL::Span<Handle<BufferView>> buffers, uint32_t arrayOffset = 0) = 0;

    /// @brief Binds an image resource to the provided binding index and offset array index.
    /// NOTE: offset + samplers count should not exceed the count of the resources decalred in the layout or the shader.
    /// @param index index of the resource binding decelration in the shader.
    /// @param samplers list of handles of an actual resources to bind.
    /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
    virtual void BindSamplers(uint32_t index, TL::Span<Handle<SamplerState>> samplers, uint32_t arrayOffset = 0) = 0;

protected:
    struct BindingData
    {
        uint32_t                          index;
        uint32_t                          bindArrayStartOffset = 0;
        ShaderBindingType                 type;
        std::vector<Handle<ImageView>>    images;
        std::vector<Handle<BufferView>>   buffers;
        std::vector<Handle<SamplerState>> samplers;
    };

    Context* m_context;

    std::vector<BindingData> m_data;
};

/// @brief ShaderBindGroupAllocator allocator used to allocate shader bind group.
class ShaderBindGroupAllocator : public Object
{
public:
    virtual ~ShaderBindGroupAllocator() = default;

    /// @brief Allocates a new shader binding group object.
    /// @param layout layout of the shader binding group resources
    virtual Handle<ShaderBindGroup> Allocate(const ShaderBindGroupLayout& layout) = 0;

    /// @brief Frees the given shader binding group object.
    virtual void Free(Handle<ShaderBindGroup> shaderBindGroup) = 0;
};

}  // namespace RHI