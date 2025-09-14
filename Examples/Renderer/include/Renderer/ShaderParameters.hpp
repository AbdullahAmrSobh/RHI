#pragma once

#include "Renderer/Resources.hpp"

#include <RHI/Resources.hpp>

namespace Engine
{
    // template<typename T>
    // concept BindableBuffer = requires(T t) {
    //     { t.getBuffer() } -> std::same_as<RHI::Buffer*>;
    //     { t.getOffset() } -> std::same_as<size_t>;
    // };

    template<typename T>
    struct ConstantBufferBinding
    {
        RHI::BufferBindingInfo bindingInfo;
        bool                   m_dirty;

        template<typename CB>
        inline auto& operator=(CB buffer)
        {
            auto newBuffer = buffer.getBuffer();
            auto newOffset = buffer.getOffset();
            if (bindingInfo.buffer != newBuffer || bindingInfo.offset != newOffset)
            {
                bindingInfo.buffer = newBuffer;
                bindingInfo.offset = newOffset;
                m_dirty = true;
            }
            return *this;
        }
    };

    template<typename T>
    struct StructuredBufferBinding
    {
        RHI::BufferBindingInfo bindingInfo;
        bool                   m_dirty;

        inline auto& operator=(T buffer)
        {
            auto newBuffer = buffer.getBuffer();
            auto newOffset = buffer.getOffset();
            if (bindingInfo.buffer != newBuffer || bindingInfo.offset != newOffset)
            {
                bindingInfo.buffer = newBuffer;
                bindingInfo.offset = newOffset;
                m_dirty = true;
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
                m_dirty = true;
            }
            return *this;
        }
    };
} // namespace Engine