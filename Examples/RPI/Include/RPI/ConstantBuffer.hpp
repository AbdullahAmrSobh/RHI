#pragma once

#include "RPI/Result.hpp"

#include <RHI/RHI.hpp>

namespace Examples::RPI
{
    template<typename StructureType>
    class ConstantBuffer
    {
    public:
        ConstantBuffer();
        ~ConstantBuffer();

        StructureType* Get();

        StructureType* Get(uint32_t index);

        StructureType* operator->();

        StructureType* operator[](uint32_t index);

        uint32_t GetElementsCount() const;

        void Update();

    private:
        RHI::BufferSubregion m_subregion;
        RHI::Handle<RHI::Buffer> m_buffer;
        RHI::DeviceMemoryPtr m_mappedPtr;
    };
} // namespace Examples::RPI