#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

class ISampler;
class ITexture;
class ITextureView;
class IBuffer;
class IBufferView;

class IDescriptorSet
{
public:
    virtual ~IDescriptorSet() = default;
    
    virtual void CommitUpdates() = 0;

public:
    struct TextureBindingDesc
    {
        ITextureView*  pTextureView;
        ETextureLayout layout;
    };
    
    struct BufferBindingDesc
    {
        IBuffer* pBuffer;
        size_t   offset;
        size_t   range;
    };
    
    struct DescriptorReference
    {
        uint32_t binding;
        uint32_t arrayIndex;
    };
    
    // Shader Resource View (SRV) binds operations.

    inline IDescriptorSet& BindSampler(const DescriptorReference descriptorReference, const ISampler& sampler)
    {
        m_bindings.emplace_back(descriptorReference, sampler);
        return *this;
    }
    inline IDescriptorSet& BindSampler(const DescriptorReference descriptorReference, ArrayView<const ISampler*> samplers)
    {
        m_bindings.emplace_back(descriptorReference, samplers);
        return *this;
    }

    inline IDescriptorSet& BindTextureView(const DescriptorReference descriptorReference, const TextureBindingDesc& textureView)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, textureView);
        return *this;
    }
    inline IDescriptorSet& BindTextureViewArray(const DescriptorReference descriptorReference, ArrayView<const TextureBindingDesc*> textureViews)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, textureViews);
        return *this;
    }
    
    // Todo - add support for input attachment descriptor types.
    // inline IDescriptorSet& BindInputAttachment(const DescriptorReference descriptorReference, const TextureBindingDesc& textureView);
    // inline IDescriptorSet& BindInputAttachmentArray(const DescriptorReference descriptorReference, ArrayView<const TextureBindingDesc*> textureViews);
    
    inline IDescriptorSet& BindUniformBuffer(const DescriptorReference descriptorReference, const BufferBindingDesc& buffer)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, buffer);
        return *this;
    }
    inline IDescriptorSet& BindUniformBufferArray(const DescriptorReference descriptorReference, ArrayView<const BufferBindingDesc*> buffers)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, buffers);
        return *this;
    }

    inline IDescriptorSet& BindUniformTexelBuffer(const DescriptorReference descriptorReference, const IBufferView& bufferView)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, bufferView);
        return *this;
    }
    
    inline IDescriptorSet& BindUniformTexelBufferArray(const DescriptorReference descriptorReference, ArrayView<const IBufferView>* bufferViews)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::ReadOnly, bufferViews);
        return *this;
    }

    // Unorederd EDescriptorAccessType View (UAV) binds operations.
    
    inline IDescriptorSet& BindStorageTexture(const DescriptorReference descriptorReference, const ITexture& texture)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::Unoredered, texture);
        return *this;
    }

    inline IDescriptorSet& BindStorageBuffer(const DescriptorReference descriptorReference, const IBuffer& buffer)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::Unoredered, buffer);
        return *this;
    }

    inline IDescriptorSet& BindTexelBuffer(const DescriptorReference descriptorReference, const IBufferView& bufferView)
    {
        m_bindings.emplace_back(descriptorReference, EDescriptorAccessType::Unoredered, bufferView);
        return *this;
    }
    
    // Copy Descriptors

    inline IDescriptorSet& CopyDescriptor(const DescriptorReference descriptorReference, const DescriptorReference sourceDescriptorReference)
    {
        m_copyDescriptors.emplace_back(descriptorReference, sourceDescriptorReference);
        return *this;
    }

public:
    struct BindingDesc
    {
        explicit BindingDesc(const DescriptorReference reference, const ISampler& sampler)
            : descriptorReference(reference)
            , type(EDescriptorType::Sampler)
            , accessType(EDescriptorAccessType::Undefined)
            , elementsCount(1)
            , pSamplers(&sampler)
        {
        }

        explicit BindingDesc(const DescriptorReference reference, const ArrayView<const ISampler*> sampler)
            : descriptorReference(reference)
            , type(EDescriptorType::Sampler)
            , accessType(EDescriptorAccessType::Undefined)
            , elementsCount(static_cast<uint32_t>(sampler.size()))
            , pSamplers(*sampler.begin())
        {
        }
        
        explicit BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const TextureBindingDesc& bindingDesc)
            : descriptorReference(reference)
            , type(EDescriptorType::Texture)
            , accessType(access)
            , elementsCount(1)
            , pTextureBindings(&bindingDesc)
        {
        }

        explicit BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const ArrayView<const TextureBindingDesc*> bindingDescs)
            : descriptorReference(reference)
            , type(EDescriptorType::Texture)
            , accessType(access)
            , elementsCount(static_cast<uint32_t>(bindingDescs.size()))
            , pTextureBindings(*bindingDescs.begin())
        {
        }
        
        explicit BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const BufferBindingDesc& bufferBinding)
            : descriptorReference(reference)
            , type(EDescriptorType::UniformBuffer)
            , accessType(access)
            , elementsCount(1)
            , pBufferBindings(&bufferBinding)
        {
        }

        explicit BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const ArrayView<const BufferBindingDesc*> bufferBindings)
            : descriptorReference(reference)
            , type(EDescriptorType::UniformBuffer)
            , accessType(access)
            , elementsCount(static_cast<uint32_t>(bufferBindings.size()))
            , pBufferBindings(*bufferBindings.begin())
        {
        }
        
        explicit BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const IBufferView& bufferView)
            : descriptorReference(reference)
            , type(EDescriptorType::TexelBuffer)
            , accessType(access)
            , elementsCount(1)
            , pBufferViews(&bufferView)
        {
        }

        explicit BindingDesc(const DescriptorReference reference, EDescriptorAccessType access, const ArrayView<const IBufferView*> bufferViews)
            : descriptorReference(reference)
            , type(EDescriptorType::TexelBuffer)
            , accessType(access)
            , elementsCount(static_cast<uint32_t>(bufferViews.size()))
            , pBufferViews(*bufferViews.begin())
        {
        }

        const DescriptorReference descriptorReference;
        EDescriptorType           type;
        EDescriptorAccessType     accessType;
        uint32_t                  elementsCount;
        union
        {
            const ISampler*           pSamplers;
            const TextureBindingDesc* pTextureBindings;
            const BufferBindingDesc*  pBufferBindings;
            const IBufferView*        pBufferViews;
        };
    };
    
    struct CopyDescriptorDesc
    {
        CopyDescriptorDesc(const DescriptorReference destination, const DescriptorReference source)
            : destination(destination)
            , source(source)
        {
        }
        
        const DescriptorReference destination;
        const DescriptorReference source;
    };
    
    std::vector<BindingDesc>        m_bindings;
    std::vector<CopyDescriptorDesc> m_copyDescriptors;

};
using DescriptorSetPtr = Unique<IDescriptorSet>;

} // namespace RHI
