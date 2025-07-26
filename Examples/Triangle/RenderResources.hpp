#pragma once

#include <RHI/RenderGraph.hpp>

namespace neon
{
    class BufferSuballocation
    {
    public:
    };

    class BufferAllocator
    {
    public:
        BufferSuballocation allocate(size_t size, size_t alignment);
        BufferSuballocation reallocate(BufferSuballocation, size_t newSize);
        void                release(BufferSuballocation allocation);
        void                grow();
    };

    ///
    //

    template<typename T>
    class ConstantBuffer
    {
    };

    template<typename T>
    class StructuredBuffer
    {
    };

    template<typename T>
    class ShaderParameters
    {
    public:
    };

    class PresentationViewport
    {
    public:
    };

    class Renderer
    {
    public:
        void FrameBegin();
        void FrameEnd();

        PresentationViewport* CreateViewport();
        void                  DestroyViewport(PresentationViewport* viewport);

        template<typename T>
        ConstantBuffer<T> CreateConstantBuffer();

        template<typename T>
        StructuredBuffer<T> CreateStructuredbuffer();

        template<typename T>
        ShaderParameters<T> CreateShaderParam();

        template<typename T>
        void UpdateConstantBuffer(RHI::RenderGraph& rg, ConstantBuffer<T> buffer, TL::Span<const T> content);
        template<typename T>
        void UpdateStructuredBuffer(RHI::RenderGraph& rg, StructuredBuffer<T> buffer, TL::Span<const T> content);
        template<typename T>
        void UpdateStructuredBuffer(RHI::RenderGraph& rg, StructuredBuffer<T> buffer, size_t offset, TL::Span<const T> content);

    private:
        RHI::Device*      m_device;
        RHI::RenderGraph* m_renderGraph;
    };
}; // namespace neon