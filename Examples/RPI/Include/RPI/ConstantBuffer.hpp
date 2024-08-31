#pragma once

#include "RPI/Result.hpp"

#include <RHI/RHI.hpp>

namespace Examples::RPI
{
    template<typename StructureType>
    class ConstantBuffer
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
    ResultCode ConstantBuffer<StructureType>::Init(RHI::Context& context)
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
    void ConstantBuffer<StructureType>::Shutdown(RHI::Context& context)
    {
        m_context->DestroyBuffer(m_buffer);
    }

    template<typename StructureType>
    StructureType* ConstantBuffer<StructureType>::Get()
    {
        return &m_structure;
    }

    template<typename StructureType>
    StructureType* ConstantBuffer<StructureType>::Get(uint32_t index)
    {
        return &m_structure + index;
    }

    template<typename StructureType>
    StructureType* ConstantBuffer<StructureType>::operator->()
    {
        return &m_structure;
    }

    template<typename StructureType>
    StructureType* ConstantBuffer<StructureType>::operator[](uint32_t index)
    {
        return &m_structure + index;
    }

    template<typename StructureType>
    uint32_t ConstantBuffer<StructureType>::GetElementsCount() const
    {
        return m_elementsSize;
    }

    template<typename StructureType>
    void ConstantBuffer<StructureType>::Update()
    {
        auto ptr = m_context->MapBuffer(m_buffer);
        memcpy(ptr, &m_structure, sizeof(StructureType));
        m_context->UnmapBuffer(m_buffer);
    }

} // namespace Examples::RPI