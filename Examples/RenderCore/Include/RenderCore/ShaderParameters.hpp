#pragma once

#include <RenderCore/Resources.hpp>

#include "Renderer/PipelineLibrary.hpp"

#include <RHI/Resources.hpp>

namespace Engine
{
    template<typename T>
    struct ConstantBufferBinding
    {
        RHI::BufferBindingInfo bindingInfo;
        bool                   m_dirty = true;

        inline auto& operator=(const GPUArray<T>& buffer)
        {
            auto binding = buffer.getBinding();
            if (bindingInfo.buffer != binding.buffer || bindingInfo.offset != binding.offset)
            {
                bindingInfo.buffer = binding.buffer;
                bindingInfo.offset = binding.offset;
                bindingInfo.range  = binding.range;
                m_dirty            = true;
            }
            return *this;
        }

        inline auto& operator=(const Buffer<T>& buffer)
        {
            auto binding = buffer.getBinding();
            if (bindingInfo.buffer != binding.buffer || bindingInfo.offset != binding.offset)
            {
                bindingInfo.buffer = binding.buffer;
                bindingInfo.offset = binding.offset;
                bindingInfo.range  = binding.range;
                m_dirty            = true;
            }
            return *this;
        }

        inline auto& operator=(RHI::BufferBindingInfo binding)
        {
            if (bindingInfo.buffer != binding.buffer || bindingInfo.offset != binding.offset)
            {
                bindingInfo.buffer = binding.buffer;
                bindingInfo.offset = binding.offset;
                bindingInfo.range  = binding.range;
                m_dirty            = true;
            }
            return *this;
        }
    };

    template<typename T>
    struct StructuredBufferBinding
    {
        RHI::BufferBindingInfo bindingInfo;
        bool                   m_dirty = true;

        inline auto& operator=(const GPUArray<T>& buffer)
        {
            auto binding = buffer.getBinding();
            if (bindingInfo.buffer != binding.buffer || bindingInfo.offset != binding.offset)
            {
                bindingInfo.buffer = binding.buffer;
                bindingInfo.offset = binding.offset;
                bindingInfo.range  = binding.range;
                m_dirty            = true;
            }
            return *this;
        }

        inline auto& operator=(const Buffer<T>& buffer)
        {
            auto binding = buffer.getBinding();
            if (bindingInfo.buffer != binding.buffer || bindingInfo.offset != binding.offset)
            {
                bindingInfo.buffer = binding.buffer;
                bindingInfo.offset = binding.offset;
                bindingInfo.range  = binding.range;
                m_dirty            = true;
            }
            return *this;
        }

        inline auto& operator=(const RHI::BufferBindingInfo& binding)
        {
            if (bindingInfo.buffer != binding.buffer || bindingInfo.offset != binding.offset)
            {
                bindingInfo.buffer = binding.buffer;
                bindingInfo.offset = binding.offset;
                bindingInfo.range  = binding.range;
                m_dirty            = true;
            }
            return *this;
        }
    };

    struct Texture1DBinding
    {
        RHI::Image* m_image;
        bool        m_dirty = true;

        inline auto& operator=(RHI::Image* image)
        {
            if (m_image != image)
            {
                m_image = image;
                m_dirty = true;
            }
            return *this;
        }
    };

    struct Texture1DArrayBinding
    {
        RHI::Image* m_image;
        bool        m_dirty = true;

        inline auto& operator=(RHI::Image* image)
        {
            if (m_image != image)
            {
                m_image = image;
                m_dirty = true;
            }
            return *this;
        }
    };

    struct Texture2DBinding
    {
        RHI::Image* m_image;
        bool        m_dirty = true;

        inline auto& operator=(RHI::Image* image)
        {
            if (m_image != image)
            {
                m_image = image;
                m_dirty = true;
            }
            return *this;
        }
    };

    struct Texture2DArrayBinding
    {
        RHI::Image* m_image;
        bool        m_dirty = true;

        inline auto& operator=(RHI::Image* image)
        {
            if (m_image != image)
            {
                m_image = image;
                m_dirty = true;
            }
            return *this;
        }
    };

    struct Texture3DBinding
    {
        RHI::Image* m_image;
        bool        m_dirty = true;

        inline auto& operator=(RHI::Image* image)
        {
            if (m_image != image)
            {
                m_image = image;
                m_dirty = true;
            }
            return *this;
        }
    };

    struct TextureCubeBinding
    {
        RHI::Image* m_image;
        bool        m_dirty = true;

        inline auto& operator=(RHI::Image* image)
        {
            if (m_image != image)
            {
                m_image = image;
                m_dirty = true;
            }
            return *this;
        }
    };

    struct SamplerBinding
    {
        RHI::Sampler* m_sampler;
        bool          m_dirty = true;

        inline auto& operator=(RHI::Sampler* sampler)
        {
            if (m_sampler != sampler)
            {
                m_sampler = sampler;
                m_dirty   = true;
            }
            return *this;
        }
    };

    template<typename T>
    class ShaderBindGroup final : public T
    {
    public:
        void init(RHI::Device* device, uint32_t bindlessArrayCount)
        {
            m_bindGroup = device->CreateBindGroup(RHI::BindGroupCreateInfo{
                .name               = typeid(T).name(),
                .layout             = PipelineLibrary::ptr->acquireLayout<T>(),
                .bindlessArrayCount = bindlessArrayCount,
            });
        }

        void shutdown(RHI::Device* device)
        {
            device->DestroyBindGroup(m_bindGroup);
        }

        void update(RHI::Device* device)
        {
            T::updateBindGroup(device, m_bindGroup);
        }

        RHI::BindGroupBindingInfo bind(uint32_t dynamicOffset) const
        {
            return {
                .bindGroup      = m_bindGroup,
                .dynamicOffsets = dynamicOffset,
            };
        }

        RHI::BindGroupBindingInfo bind() const
        {
            return {
                .bindGroup      = m_bindGroup,
                .dynamicOffsets = {},
            };
        }
    private:
        RHI::BindGroup*                     m_bindGroup;
    };
} // namespace Engine