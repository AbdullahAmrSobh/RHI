#pragma once

#include "RPI/Result.hpp"

#include <RHI/RHI.hpp>

namespace Examples::RPI
{
    // class BufferSuballocator
    // {

    // }

    // 1 needs buffer suballocate class (could be used to suballocate vertex/index/uniform/storage...etc all sorts of buffers) // take alignmentOffsets into account
    // suballocate from a big unifrom buffer using minUniformBufferOffsetAlignment

    template<typename StructureType>
    class ConstantBuffer
    {
    public:

    };

    template<typename StructureType>
    class ConstantBufferView
    {
    public:
        ResultCode Init(RHI::Context& context);
        void Shutdown(RHI::Context& context);

        RHI::Handle<RHI::Buffer> GetBuffer() { return m_buffer; }

        StructureType* Get();

        StructureType* Get(uint32_t index);

        StructureType* operator->();

        StructureType* operator[](uint32_t index);

        uint32_t GetElementsCount() const;

        void Update();

    private:
        RHI::Context* m_context;
        RHI::BufferSubregion m_subregion;
        RHI::Handle<RHI::Buffer> m_buffer;
        StructureType m_structure;
        uint32_t m_elementsSize;
    };

    template<typename StructureType>
    ResultCode ConstantBufferView<StructureType>::Init(RHI::Context& context)
    {
        m_context = &context;

        RHI::BufferCreateInfo bufferCI{};
        bufferCI.name = "UniformBuffer";
        bufferCI.heapType = RHI::MemoryType::GPULocal;
        bufferCI.usageFlags = RHI::BufferUsage::Uniform;
        bufferCI.byteSize = sizeof(StructureType);
        auto [buffer, result] = context.CreateBuffer(bufferCI);
        m_buffer = buffer;
        TL_ASSERT(RHI::IsSucess(result));

        return ResultCode::Sucess;
    }

    template<typename StructureType>
    void ConstantBufferView<StructureType>::Shutdown(RHI::Context& context)
    {
        m_context->DestroyBuffer(m_buffer);
    }

    template<typename StructureType>
    StructureType* ConstantBufferView<StructureType>::Get()
    {
        return &m_structure;
    }

    template<typename StructureType>
    StructureType* ConstantBufferView<StructureType>::Get(uint32_t index)
    {
        return &m_structure + index;
    }

    template<typename StructureType>
    StructureType* ConstantBufferView<StructureType>::operator->()
    {
        return &m_structure;
    }

    template<typename StructureType>
    StructureType* ConstantBufferView<StructureType>::operator[](uint32_t index)
    {
        return &m_structure + index;
    }

    template<typename StructureType>
    uint32_t ConstantBufferView<StructureType>::GetElementsCount() const
    {
        return m_elementsSize;
    }

    template<typename StructureType>
    void ConstantBufferView<StructureType>::Update()
    {
        auto ptr = m_context->MapBuffer(m_buffer);
        memcpy(ptr, &m_structure, sizeof(StructureType));
        m_context->UnmapBuffer(m_buffer);
    }

} // namespace Examples::RPI