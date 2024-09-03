#include "Assets/Buffer.hpp"

namespace Examples::Assets
{
    Buffer::Buffer(Buffer&& other)
    {
        m_name = std::move(other.m_name);
        std::swap(m_format, other.m_format);
        std::swap(m_elementsCount, other.m_elementsCount);
        std::swap(m_strideSize, other.m_strideSize);
        std::swap(m_alignment, other.m_alignment);
        std::swap(m_sizeBytes, other.m_sizeBytes);
        std::swap(m_data, other.m_data);
    }

    Buffer::~Buffer()
    {
        if (m_data.ptr)
        {
            TL::Allocator::Release(m_data, m_alignment);
        }
    }

    Buffer& Buffer::operator=(Buffer&& other)
    {
        m_name = std::move(other.m_name);
        std::swap(m_format, other.m_format);
        std::swap(m_elementsCount, other.m_elementsCount);
        std::swap(m_strideSize, other.m_strideSize);
        std::swap(m_alignment, other.m_alignment);
        std::swap(m_sizeBytes, other.m_sizeBytes);
        std::swap(m_data, other.m_data);
        return *this;
    }

    const char* Buffer::GetName() const
    {
        return m_name.c_str();
    }

    uint32_t Buffer::GetElementsCount() const
    {
        return m_elementsCount;
    }

    uint32_t Buffer::GetStrideSize() const
    {
        return m_strideSize;
    }

    TL::Block Buffer::GetData() const
    {
        return m_data;
    }

} // namespace Examples::Assets