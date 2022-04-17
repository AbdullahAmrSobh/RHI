#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

class ISampler;
class IImage;
class IImageView;
class IBuffer;
class IBufferView;

struct ImageBindingDesc
{
    ImageBindingDesc() = default;
    ImageBindingDesc(IImageView& image, EImageLayout layout)
        : pImageView(&image)
        , layout(layout)
    {
    }

    IImageView*  pImageView;
    EImageLayout layout;
};

struct BufferBindingDesc
{
    BufferBindingDesc() = default;
    BufferBindingDesc(IBuffer& buffer, uint32_t offset, uint32_t range)
        : pBuffer(&buffer)
        , offset(offset)
        , range(range)
    {
    }

    IBuffer* pBuffer;
    size_t   offset;
    size_t   range;
};

struct DescriptorReference
{
    DescriptorReference() = default;
    DescriptorReference(uint32_t binding = 0, uint32_t arrayIndex = 0)
        : binding(binding)
        , arrayIndex(arrayIndex)
    {
    }

    uint32_t binding;
    uint32_t arrayIndex;
};

struct CopyDescriptorDesc
{
    CopyDescriptorDesc()
        : destination(0, 0)
        , source(0, 0)
    {
    }

    CopyDescriptorDesc(DescriptorReference destination, DescriptorReference source)
        : destination(destination)
        , source(source)
    {
    }

    DescriptorReference destination;
    DescriptorReference source;
};

class IDescriptorSet
{
public:
    virtual ~IDescriptorSet() = default;

    virtual void CommitUpdates() = 0;

public:
    // Shader Resource View (SRV) binds operations.

    inline void BindSampler(const DescriptorReference descriptorReference, const ISampler& sampler)
    {
        m_bindings.emplace_back(descriptorReference, sampler);
    }

    inline void BindSampler(const DescriptorReference descriptorReference, Span<const ISampler*> samplers)
    {
        m_bindings.emplace_back(descriptorReference, samplers);
    }

    inline void BindImageView(const DescriptorReference descriptorReference, const ImageBindingDesc& ImageView)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, ImageView);
    }

    inline void BindImageViewArray(const DescriptorReference descriptorReference, Span<const ImageBindingDesc*> ImageViews)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, ImageViews);
    }

    // Todo - add support for input attachment descriptor types.
    // inline void BindInputAttachment(const DescriptorReference descriptorReference, const ImageBindingDesc& ImageView);
    // inline void BindInputAttachmentArray(const DescriptorReference descriptorReference, Span<const ImageBindingDesc*> ImageViews);

    inline void BindUniformBuffer(const DescriptorReference descriptorReference, const BufferBindingDesc& buffer)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, buffer);
    }
    inline void BindUniformBufferArray(const DescriptorReference descriptorReference, Span<const BufferBindingDesc*> buffers)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, buffers);
    }

    inline void BindUniformTexelBuffer(const DescriptorReference descriptorReference, const IBufferView& bufferView)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, bufferView);
    }

    inline void BindUniformTexelBufferArray(const DescriptorReference descriptorReference, const Span<const IBufferView*> bufferViews)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, bufferViews);
    }

    // Unorederd EDescriptorAccessType View (UAV) binds operations.

    inline void BindStorageImage(const DescriptorReference descriptorReference, const ImageBindingDesc& imageBindingDesc)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::Unoredered, imageBindingDesc);
    }

    inline void BindStorageBuffer(const DescriptorReference descriptorReference, const BufferBindingDesc& bufferBindingDesc)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::Unoredered, bufferBindingDesc);
    }

    inline void BindTexelBuffer(const DescriptorReference descriptorReference, const IBufferView& bufferView)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::Unoredered, bufferView);
    }

    // Copy Descriptors

    inline void CopyDescriptor(const DescriptorReference descriptorReference, const DescriptorReference sourceDescriptorReference)
    {
        m_copyDescriptors.emplace_back(descriptorReference, sourceDescriptorReference);
    }

public:
    struct BindingDesc
    {
        // Sampler constructor
        BindingDesc(const DescriptorReference reference, const ISampler& sampler)
            : descriptorReference(reference)
            , type(EDescriptorType::Sampler)
            , accessType(EDescriptorAccessType::ReadOnly)
            , elementsCount(1)
            , pSamplers(&sampler)
        {
        }

        BindingDesc(const DescriptorReference reference, const Span<const ISampler*> samplers)
            : descriptorReference(reference)
            , type(EDescriptorType::Sampler)
            , accessType(EDescriptorAccessType::ReadOnly)
            , elementsCount(1)
            , pSamplers(samplers.front())
        {
        }

        // Image constructor
        BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const ImageBindingDesc& bindingDesc)
            : descriptorReference(reference)
            , type(EDescriptorType::Image)
            , accessType(access)
            , elementsCount(1)
            , pImageBindings(&bindingDesc)
        {
        }

        BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const Span<const ImageBindingDesc*> bindingDescs)
            : descriptorReference(reference)
            , type(EDescriptorType::Image)
            , accessType(access)
            , elementsCount(1)
            , pImageBindings(bindingDescs.front())
        {
        }

        // Buffer constructor
        BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const BufferBindingDesc& bufferBinding)
            : descriptorReference(reference)
            , type(EDescriptorType::UniformBuffer)
            , accessType(access)
            , elementsCount(1)
            , pBufferBindings(&bufferBinding)
        {
        }

        BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const Span<const BufferBindingDesc*> bufferBindings)
            : descriptorReference(reference)
            , type(EDescriptorType::UniformBuffer)
            , accessType(access)
            , elementsCount(1)
            , pBufferBindings(bufferBindings.front())
        {
        }

        // BufferView constructor
        BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const IBufferView& bufferView)
            : descriptorReference(reference)
            , type(EDescriptorType::TexelBuffer)
            , accessType(access)
            , elementsCount(1)
            , pBufferViews(&bufferView)
        {
        }

        BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const Span<const IBufferView*> bufferViews)
            : descriptorReference(reference)
            , type(EDescriptorType::TexelBuffer)
            , accessType(access)
            , elementsCount(1)
            , pBufferViews(bufferViews.front())
        {
        }

        const DescriptorReference descriptorReference;
        EDescriptorType           type;
        EDescriptorAccessType     accessType;
        uint32_t                  elementsCount;
        union
        {
            const ISampler*          pSamplers;
            const ImageBindingDesc*  pImageBindings;
            const BufferBindingDesc* pBufferBindings;
            const IBufferView*       pBufferViews;
        };
    };

    std::vector<BindingDesc>        m_bindings;
    std::vector<CopyDescriptorDesc> m_copyDescriptors;
};
using DescriptorSetPtr = Unique<IDescriptorSet>;

} // namespace RHI
