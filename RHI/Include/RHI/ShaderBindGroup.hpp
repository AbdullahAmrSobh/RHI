#pragma once
#include <array>
#include <variant>
#include <vector>

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
    TL::Span<const ShaderBinding> bindings;
};

/// @brief An object that groups shader resources that are bound together.
class ShaderBindGroupData final
{
public:
    ShaderBindGroupData() = default;

    /// @brief Binds an image resource to the provided binding index and offset array index.
    /// NOTE: offset + images count should not exceed the count of the resources decalred in the layout or the shader.
    /// @param index index of the resource binding decelration in the shader.
    /// @param images list of handles of an actual resources to bind.
    /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
    inline void BindImages(uint32_t index, TL::Span<const Handle<ImageView>> images, uint32_t arrayOffset = 0)
    {
        BindingData data {};
        data.index = index;
        data.resources = std::vector<Handle<ImageView>>(images.begin(), images.end());
        data.bindArrayStartOffset = arrayOffset;
        
        m_data.push_back(data);
        
        std::sort(m_data.begin(), m_data.end(), [](auto lhs, auto rhs){
            return lhs.index < rhs.index;
        });
    } 

    /// @brief Binds an image resource to the provided binding index and offset array index.
    /// NOTE: offset + buffers count should not exceed the count of the resources decalred in the layout or the shader.
    /// @param index index of the resource binding decelration in the shader.
    /// @param buffers list of handles of an actual resources to bind.
    /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
    inline void BindBuffers(uint32_t index, TL::Span<const Handle<BufferView>> buffers, uint32_t arrayOffset = 0)
    {
        BindingData data {};
        data.index = index;
        data.resources = std::vector<Handle<BufferView>>(buffers.begin(), buffers.end());
        data.bindArrayStartOffset = arrayOffset;
        
        m_data.push_back(data);
        
        std::sort(m_data.begin(), m_data.end(), [](auto lhs, auto rhs){
            return lhs.index < rhs.index;
        });
    }

    /// @brief Binds an image resource to the provided binding index and offset array index.
    /// NOTE: offset + samplers count should not exceed the count of the resources decalred in the layout or the shader.
    /// @param index index of the resource binding decelration in the shader.
    /// @param samplers list of handles of an actual resources to bind.
    /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
    inline void BindSamplers(uint32_t index, TL::Span<const Handle<Sampler>> samplers, uint32_t arrayOffset = 0)
    {
        BindingData data {};
        data.index = index;
        data.resources = std::vector<Handle<Sampler>>(samplers.begin(), samplers.end());
        data.bindArrayStartOffset = arrayOffset;
        
        m_data.push_back(data);
        
        std::sort(m_data.begin(), m_data.end(), [](auto lhs, auto rhs){
            return lhs.index < rhs.index;
        });
    }

private:
    using BindingResourceList = std::variant<std::vector<Handle<ImageView>>,
                                             std::vector<Handle<BufferView>>,
                                             std::vector<Handle<Sampler>>>;

    struct BindingData
    {
        uint32_t index;

        uint32_t bindArrayStartOffset = 0;

        BindingResourceList resources;
    };

    std::vector<BindingData> m_data;

    uint32_t m_imagesCount;

    uint32_t m_bufferCount;

    uint32_t m_samplerCount;
};

class ShaderBindGroup
{
};

class ShaderBindGroupAllocator : public Object
{
public:
    using Object::Object;
    virtual ~ShaderBindGroupAllocator() = default;

    virtual std::vector<Handle<ShaderBindGroup>> AllocateShaderBindGroups(TL::Span<ShaderBindGroupLayout> layouts) = 0;

    virtual void Free(TL::Span<Handle<ShaderBindGroup>> groups) = 0;

    virtual void Update(Handle<ShaderBindGroup> group, const ShaderBindGroupData& data) = 0;
};

}  // namespace RHI