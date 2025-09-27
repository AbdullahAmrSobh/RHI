#pragma once

#include "Renderer/Resources.hpp"

#include <RHI/Resources.hpp>

namespace Engine
{
    template<typename T>
    struct ConstantBufferBinding
    {
        RHI::BufferBindingInfo bindingInfo;
        bool                   m_dirty;

        inline auto& operator=(GPUArray<T> buffer)
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

        inline auto& operator=(Buffer<T> buffer)
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
        bool                   m_dirty;

        inline auto& operator=(GPUArray<T> buffer)
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

        inline auto& operator=(Buffer<T> buffer)
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

    struct Texture1DBinding
    {
        RHI::Image* m_image;
        bool        m_dirty;

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
        bool        m_dirty;

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
        bool        m_dirty;

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
        bool        m_dirty;

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
        bool        m_dirty;

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
        bool        m_dirty;

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
        bool          m_dirty;

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
} // namespace Engine