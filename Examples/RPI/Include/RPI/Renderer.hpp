#pragma once

#include "RPI/Result.hpp"

#include <RHI/RHI.hpp>

namespace Examples
{
    class Window;
}

namespace Examples::RPI
{
    template<typename T>
    using Handle = RHI::Handle<T>;

    inline static constexpr uint32_t BufferedFramesCount = 2;

    template<typename T>
    class RingBuffer
    {
    public:
        T& Get()
        {
            return m_buffer[m_index];
        }

        void advance()
        {
            m_index = (m_index + 1) % BufferedFramesCount;
        }

        T* begin()
        {
            return m_buffer;
        }

        T* end()
        {
            return m_buffer + 2;
        }

    private:
        T m_buffer[BufferedFramesCount]; ///< The internal buffer storage.
        size_t m_index = 0;              ///< The current index in the buffer.
    };

    class Mesh
    {
    public:
        uint32_t m_elementsCount;
        Handle<RHI::Buffer> m_indexIB;
        Handle<RHI::Buffer> m_positionVB;
        Handle<RHI::Buffer> m_normalVB;
    };

    class Renderer
    {
    public:
        Renderer() = default;
        ~Renderer() = default;

        ResultCode Init(const Window& window);
        void Shutdown();
        void Render(TL::Span<RPI::Mesh> meshes);

        virtual ResultCode OnInit() = 0;
        virtual void OnShutdown() = 0;
        virtual void OnRender(TL::Span<RPI::Mesh> meshes) = 0;

        // protected:
        const Window* m_window;

        TL::Ptr<RHI::Context> m_context;
        TL::Ptr<RHI::Swapchain> m_swapchain;
        TL::Ptr<RHI::RenderGraph> m_renderGraph;

        Handle<RHI::ImageAttachment> m_outputAttachment;

        struct FrameContext
        {
            TL::Ptr<RHI::Fence> m_fence;
            TL::Ptr<RHI::CommandPool> m_commandPool;
        };

        RingBuffer<FrameContext> m_frameRingbuffer;
    };
} // namespace Examples::RPI