#pragma once

#include <cstdint>
#include <span>

#include "RHI/Export.hpp"
#include "RHI/Flags.hpp"
#include "RHI/Object.hpp"

namespace RHI
{

class ImageAttachmentView;
class BufferAttachmentView;

enum class ShaderResourceType
{
    Sampler = 0,
    Image,
    Buffer,
    Count
};

struct ShaderBinding
{
    ShaderResourceType  type;
    uint32_t            arrayCount;
    Flags<ShaderStages> stages;
};

struct ShaderBindingGroupLayout
{
    ShaderBinding bindings[32];
};

// List of image bind groups which should be bound.
class RHI_EXPORT ShaderBindGroup
{
public:
    inline constexpr uint32_t MaxBindingSlots = 8;

    using Object::Object;
    virtual ~ShaderBindGroup() = default;

    void SetImageView(uint32_t slot, ImageAttachmentView imageView, uint32_t arrayIndex = 0);

    void SetImageViewArray(uint32_t slot, std::span<ImageAttachmentView> imageViews, uint32_t arrayIndex = 0);

    void SetBufferView(uint32_t slot, BufferAttachmentView bufferView, uint32_t arrayIndex = 0);

    void SetBufferViewArray(uint32_t slot, std::span<BufferAttachmentView> bufferViews, uint32_t arrayIndex = 0);

    void SetSampler(uint32_t slot, Sampler sampler, uint32_t arrayIndex = 0);

    void SetSamplerArray(uint32_t slot, std::span<Sampler> samplers, uint32_t arrayIndex = 0);

private:
    struct Binding
    {
        ShaderResourceType type;

        union
        {
            std::span<ImageAttachmentView>  imageAttachment;
            std::span<BufferAttachmentView> bufferAttachment;
            std::span<Sampler>              samplers;
        };
    };

    using BindingsList = std::array<Binding, MaxBindingSlots>;
    using BindingTable = std::array<BindingsList, ShaderResourceType::Count>;

    BindingTable m_bindingTable;
};

}  // namespace RHI