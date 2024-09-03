#pragma once

#include "Assets/Export.hpp"
#include "Assets/Definitions.hpp"

#include <glm/glm.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>
#include <TL/Block.hpp>
#include <TL/Serialization/Binary.hpp>

namespace Examples::Assets
{
    /// @brief Represents a buffer used for storing asset data.
    ///
    /// The Buffer class manages a block of memory, typically for storing vertex data,
    /// indices, or other types of binary data used in rendering or asset management.
    class ASSETS_EXPORT Buffer
    {
    public:
        Buffer() = default;

        /// @brief Constructs a Buffer from a span of data.
        ///
        /// This constructor initializes the buffer from a span of elements,
        /// computing the format, element count, stride size, and alignment based
        /// on the type of the data provided.
        ///
        /// @tparam T The type of the elements in the span.
        /// @param name The name of the buffer.
        /// @param data A span of elements to initialize the buffer.
        template<typename T>
        Buffer(const char* name, TL::Span<const T> data)
            : m_name(name)
            , m_format(GetVertexFormat<T>())
            , m_elementsCount(static_cast<uint32_t>(data.size()))
            , m_strideSize(static_cast<uint32_t>(sizeof(T)))
            , m_alignment(alignof(T))
            , m_sizeBytes(data.size_bytes())
            , m_data()
        {
            m_data = TL::Allocator::Allocate(data.size_bytes(), alignof(T));
            memcpy(m_data.ptr, data.data(), data.size_bytes());
        }

        ~Buffer();

        Buffer(Buffer&& other);
        Buffer(const Buffer& other) = delete;

        Buffer& operator=(Buffer&& other);
        Buffer& operator=(const Buffer& other) = delete;

        /// @brief Gets the name of the buffer.
        /// @return The name of the buffer.
        const char* GetName() const;

        /// @brief Gets the number of elements in the buffer.
        /// @return The number of elements in the buffer.
        uint32_t GetElementsCount() const;

        /// @brief Gets the stride size of the buffer.
        /// @return The stride size of the buffer.
        uint32_t GetStrideSize() const;

        /// @brief Gets the data block of the buffer.
        /// @return The block of data stored in the buffer.
        TL::Block GetData() const;

        /// @brief Returns a span of the buffer's data as a specific type.
        ///
        /// This method provides a type-safe way to access the buffer's data. It first checks
        /// that the buffer's format and alignment match the expected format and alignment for
        /// the specified type `T`. If these checks pass, it returns a span of the data as `T`.
        ///
        /// @note: for custom formats, please be carefull with your
        ///
        /// @tparam T The type of elements the span should represent.
        /// @return A span of the buffer's data as `T`.
        /// @note The function asserts that the buffer's format matches `GetVertexFormat<T>()`
        ///       and that the buffer's alignment matches `alignof(T)`. If these conditions
        ///       are not met, the assertions will fail.
        /// @warning The caller must ensure that the buffer's format and alignment are correct
        ///          for the specified type `T` to avoid potential undefined behavior.
        template<typename T>
        TL::Span<const T> GetData() const
        {
            TL_ASSERT(m_format == GetVertexFormat<T>(), "Buffer format does not match the expected format for type T.");
            TL_ASSERT(m_alignment == alignof(T), "Buffer alignment does not match the alignment of type T.");
            return TL::Span<const T>(reinterpret_cast<T*>(m_data.ptr), m_elementsCount);
        }

        template<typename T>
        TL::Span<T> GetData() const
        {
            TL_ASSERT(m_format == GetVertexFormat<T>(), "Buffer format does not match the expected format for type T.");
            TL_ASSERT(m_alignment == alignof(T), "Buffer alignment does not match the alignment of type T.");
            return TL::Span<T>(reinterpret_cast<T*>(m_data.ptr), m_elementsCount);
        }

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, m_name);
            TL::Encode(archive, m_format);
            TL::Encode(archive, m_elementsCount);
            TL::Encode(archive, m_strideSize);
            TL::Encode(archive, m_alignment);
            TL::Encode(archive, m_sizeBytes);
            TL::Encode(archive, m_data);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, m_name);
            TL::Decode(archive, m_format);
            TL::Decode(archive, m_elementsCount);
            TL::Decode(archive, m_strideSize);
            TL::Decode(archive, m_alignment);
            TL::Decode(archive, m_sizeBytes);
            m_data = TL::Allocator::Allocate(m_sizeBytes, m_alignment);
            TL::Decode(archive, m_data);
        }

    private:
        /// @brief The name of the buffer.
        TL::String m_name;

        /// @brief The format of the vertices in the buffer.
        VertexFormat m_format;

        /// @brief The number of elements in the buffer.
        uint32_t m_elementsCount;

        /// @brief The stride size of the buffer.
        uint32_t m_strideSize;

        /// @brief The alignment requirement for the buffer.
        size_t m_alignment;

        /// @brief The actual data size in bytes.
        size_t m_sizeBytes;

        /// @brief The block of data representing the buffer's contents.
        TL::Block m_data;
    };
} // namespace Examples::Assets
