#pragma once

#include "Examples-Base/Common.hpp"

#include <Examples-Base/ImGuiRenderer.hpp>

#include <RHI/RHI.hpp>

namespace Examples
{
    class Window;

    class Scene;

    class Renderer
    {
    public:
        Renderer();

        virtual ~Renderer();

        ResultCode Init(const class Window& window);

        void Shutdown();

        void Render(const Scene& scene);

        template<typename T>
        RHI::Result<Handle<RHI::Image>> CreateImageWithData(const RHI::ImageCreateInfo& createInfo, TL2::Span<const T> content);

        template<typename T>
        RHI::Result<Handle<RHI::Buffer>> CreateBufferWithData(Flags<RHI::BufferUsage> usageFlags, TL2::Span<const T> content);

        Handle<RHI::Image> CreateImage(const char* filePath);

        Ptr<Scene> CreateScene();

        virtual ResultCode OnInit() = 0;

        virtual void OnShutdown() = 0;

        virtual void OnRender(const Scene& scene) = 0;

    // protected:
        const Window* m_window;

        Ptr<RHI::Context> m_context;
        Ptr<RHI::Swapchain> m_swapchain;
        Ptr<RHI::CommandPool> m_commandPool[2];
        Ptr<RHI::Fence> m_frameFence[2];

        // Ptr<>
    };

    template<typename T>
    inline RHI::Result<Handle<RHI::Image>> Renderer::CreateImageWithData(const RHI::ImageCreateInfo& createInfo, TL2::Span<const T> content)
    {
        auto [handle, result] = m_context->CreateImage(createInfo);

        if (result != RHI::ResultCode::Success)
            return result;

        auto stagingBuffer = m_context->AllocateTempBuffer(content.size_bytes());
        memcpy(stagingBuffer.ptr, content.data(), content.size_bytes());

        RHI::ImageSubresourceLayers subresources{};
        subresources.imageAspects = RHI::GetFormatAspects(createInfo.format);
        subresources.arrayCount = createInfo.arrayCount;
        subresources.mipLevel = createInfo.mipLevels;
        m_context->StageResourceWrite(handle, subresources, stagingBuffer.buffer, stagingBuffer.offset);

        return handle;
    }

    template<typename T>
    inline RHI::Result<Handle<RHI::Buffer>> Renderer::CreateBufferWithData(Flags<RHI::BufferUsage> usageFlags, TL2::Span<const T> content)
    {
        RHI::BufferCreateInfo createInfo{};
        createInfo.byteSize = content.size_bytes();
        createInfo.usageFlags = usageFlags;
        auto [handle, result] = m_context->CreateBuffer(createInfo);

        if (result != RHI::ResultCode::Success)
            return result;

        if (content.size_bytes() <= m_context->GetLimits().stagingMemoryLimit)
        {
            auto ptr = m_context->MapBuffer(handle);
            memcpy(ptr, content.data(), content.size_bytes());
            m_context->UnmapBuffer(handle);
        }
        else
        {
            auto stagingBuffer = m_context->AllocateTempBuffer(content.size_bytes());
            memcpy(stagingBuffer.ptr, content.data(), content.size_bytes());
            m_context->StageResourceWrite(handle, 0, content.size_bytes(), stagingBuffer.buffer, stagingBuffer.offset);
        }

        return handle;
    }

} // namespace Examples