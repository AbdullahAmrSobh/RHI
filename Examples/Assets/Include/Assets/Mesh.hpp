#pragma once

#include "Assets/Export.hpp"
#include "Assets/Buffer.hpp"

#include <TL/Span.hpp>
#include <TL/Block.hpp>
#include <TL/Containers.hpp>
#include <TL/Serialization/Binary.hpp>

namespace Examples::Assets
{
    /// @brief Represents a mesh asset that holds multiple buffers and attributes.
    ///
    /// The Mesh class manages a collection of buffers and attributes, providing functions to
    /// retrieve, add, and serialize/deserialize these attributes. Each attribute is associated
    /// with a name, which is used as a key for retrieval and management.
    ///
    /// @note The class is non-copyable to ensure unique ownership of resources.
    class ASSETS_EXPORT Mesh
    {
    public:
        Mesh() = default;

        /// @brief Constructs a Mesh with a given name.
        ///
        /// @param name The name of the mesh.
        Mesh(const char* name)
            : m_name(name)
            , m_attributes()
        {
        }

        Mesh(Mesh&& other) = default;
        Mesh(const Mesh& other) = delete;
        ~Mesh() = default;

        Mesh& operator=(Mesh&& other) = default;
        Mesh& operator=(const Mesh& other) = delete;

        /// @brief Retrieves a constant pointer to a Buffer by name.
        ///
        /// @param name The name of the buffer to retrieve.
        /// @return A constant pointer to the Buffer if found, or nullptr if not found.
        const Buffer* GetBuffer(const char* name) const;

        /// @brief Retrieves a pointer to a Buffer by name.
        ///
        /// @param name The name of the buffer to retrieve.
        /// @return A pointer to the Buffer if found, or nullptr if not found.
        Buffer* GetBuffer(const char* name);

        /// @brief Retrieves an attribute as a span of the specified type.
        ///
        /// This function returns a span of the specified type `T` for the attribute identified
        /// by the given name. It assumes that the attribute's data can be interpreted as a
        /// span of `T`.
        ///
        /// @tparam T The type of elements in the span.
        /// @param name The name of the attribute to retrieve.
        /// @return A span of type `T` pointing to the attribute's data.
        /// @warning Ensure that the attribute's data matches the specified type `T`.
        template<typename T>
        TL::Span<const T> GetAttribute(const char* name) const
        {
            return GetBuffer(name)->GetData<T>();
        }

        /// @brief Adds an attribute to the mesh.
        ///
        /// This function adds a new attribute to the mesh, using the provided data. The attribute
        /// is associated with a name, and the function asserts that an attribute with the same
        /// name does not already exist.
        ///
        /// @tparam T The type of elements in the attribute data.
        /// @param name The name of the attribute.
        /// @param data A span of data to be added as an attribute.
        template<typename T>
        void AddAttribute(const char* name, TL::Span<const T> data)
        {
            TL_ASSERT(m_attributes.find(name) == m_attributes.end());
            m_attributes[name] = std::move(Buffer(name, data));
        }

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, m_name);
            TL::Encode(archive, m_attributes);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, m_name);
            TL::Decode(archive, m_attributes);
        }

    private:
        TL::String m_name;                                 ///< The name of the mesh.
        TL::UnorderedMap<TL::String, Buffer> m_attributes; ///< A map of attribute names to buffers.
    };

} // namespace Examples::Assets