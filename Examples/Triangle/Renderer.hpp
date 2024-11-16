#pragma once

#include <RHI/RHI.hpp>

namespace Examples
{
    struct BufferView
    {
        RHI::Handle<RHI::Buffer> buffer;
        RHI::BufferSubregion     subregion;

        RHI::BufferBindingInfo GetBindingInfo() const
        {
            return RHI::BufferBindingInfo{
                .buffer = buffer,
                .offset = subregion.offset,
            };
        }
    };

    class BufferPool
    {
    public:
        RHI::Handle<RHI::Buffer> m_buffer;
        size_t                   m_deviceSize;
        // OffsetAllocator::Allocator* m_allocator;

    private:
        RHI::ResultCode Init(RHI::Device& device, const RHI::BufferCreateInfo& createInfo);

        void Shutdown(RHI::Device& device);

        RHI::Result<BufferView> Allocate(size_t size);

        void Release(BufferView view);
    };

    class Mesh
    {
    public:
        enum Attribute
        {
            Index,
            Position,
            Normals,
            Texcoord0,
            Count,
        };

        struct AttributeDesc
        {
            Attribute attribute;
            TL::Block content;
        };

        void Init(RHI::Device& device);
        void Init(RHI::Device& device, TL::Span<const AttributeDesc> attributes);
        void Shutdown(RHI::Device& device);

        void Draw(RHI::CommandList& commandList);

    private:
        uint32_t   m_elementsCount;
        BufferView m_attributes[Attribute::Count];
    };

    class SceneGraph
    {
    public:
    };

    class Scene
    {
    public:
    };

    class Renderer
    {
    public:
    };
} // namespace Examples