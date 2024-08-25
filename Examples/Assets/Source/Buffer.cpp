#include "Assets/Buffer.hpp"

namespace Examples::Assets
{
    Buffer::Buffer(Buffer&& other)
    {
        this->m_name = std::move(other.m_name);
        this->m_format = std::move(other.m_format);
        this->m_elementsCount = std::move(other.m_elementsCount);
        this->m_strideSize = std::move(other.m_strideSize);
        this->m_data = std::move(other.m_data);
        other.m_data = {};
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
        this->m_name = std::move(other.m_name);
        this->m_format = std::move(other.m_format);
        this->m_elementsCount = std::move(other.m_elementsCount);
        this->m_strideSize = std::move(other.m_strideSize);
        this->m_data = std::move(other.m_data);
        other.m_data = {};
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