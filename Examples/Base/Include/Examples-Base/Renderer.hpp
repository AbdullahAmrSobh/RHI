#pragma once

#include "Examples-Base/Common.hpp"

#include <Examples-Base/ImGuiRenderer.hpp>

#include <RHI/RHI.hpp>

namespace Examples
{
    class Window;

    class Renderer
    {
    public:
        Renderer();

        virtual ~Renderer();

        ResultCode Init(const class Window& window);

        void Shutdown();

        void Render();

        virtual ResultCode OnInit() = 0;

        virtual void OnShutdown() = 0;

        virtual void OnRender() = 0;

    protected:
        const Window* m_window;

        Ptr<RHI::Context> m_context;
        Ptr<RHI::Swapchain> m_swapchain;
        Ptr<RHI::CommandPool> m_commandPool[2];
        Ptr<RHI::Fence> m_frameFence[2];
    };

    // // TODO: define in Renderer
    // template<typename T>
    // inline static Result<Handle<Image>> CreateImageWithData(Context& context, const ImageCreateInfo& createInfo, TL::Span<const T> content)
    // {
    //     auto [handle, result] = context.CreateImage(createInfo);

    //     if (result != ResultCode::Success)
    //         return result;

    //     auto stagingBuffer = context.AllocateTempBuffer(content.size_bytes());
    //     memcpy(stagingBuffer.ptr, content.data(), content.size_bytes());

    //     ImageSubresourceLayers subresources{};
    //     subresources.imageAspects = ImageAspect::Color; // todo: this should be deduced from the format
    //     subresources.arrayCount   = createInfo.arrayCount;
    //     subresources.mipLevel     = createInfo.mipLevels;
    //     context.StageResourceWrite(handle, subresources, stagingBuffer.buffer, stagingBuffer.offset);

    //     return handle;
    // }

    // template<typename T>
    // inline static Result<Handle<Buffer>> CreateBufferWithData(Context& context, Flags<BufferUsage> usageFlags, TL::Span<const T> content)
    // {
    //     BufferCreateInfo createInfo{};
    //     createInfo.byteSize   = content.size_bytes();
    //     createInfo.usageFlags = usageFlags;

    //     auto [handle, result] = context.CreateBuffer(createInfo);

    //     if (result != ResultCode::Success)
    //         return result;

    //     if (content.size_bytes() <= context.GetLimits().stagingMemoryLimit)
    //     {
    //         auto ptr = context.MapBuffer(handle);
    //         memcpy(ptr, content.data(), content.size_bytes());
    //         context.UnmapBuffer(handle);
    //     }
    //     else
    //     {
    //         auto stagingBuffer = context.AllocateTempBuffer(content.size_bytes());
    //         memcpy(stagingBuffer.ptr, content.data(), content.size_bytes());
    //         context.StageResourceWrite(handle, 0, content.size_bytes(), stagingBuffer.buffer, stagingBuffer.offset);
    //     }

    //     return handle;
    // }

} // namespace Examples