#pragma once

#include "Assets/Export.hpp"
#include "Assets/Definitions.hpp"

#include <TL/Span.hpp>
#include <TL/Containers.hpp>
#include <TL/Serialization/Binary.hpp>
#include <TL/Block.hpp>
#include <TL/UniquePtr.hpp>

namespace Examples::Assets
{
    enum class ImageExtension
    {
        DDS,
        PNG,
    };

    /// @brief Represents the dimensions of a texture.
    struct ImageSize
    {
        uint32_t width;  ///< The width of the texture.
        uint32_t height; ///< The height of the texture.
        uint32_t depth;  ///< The depth of the texture (for 3D textures).
    };

    /// @brief Represents a texture asset, containing its data and properties.
    class ASSETS_EXPORT Image
    {
        static TL::Ptr<Image> STBLoad(TL::Block block);
        static TL::Ptr<Image> DDSLoad(TL::Block block);

    public:
        /// @brief Loads from the given memory block
        static TL::Ptr<Image> Load(TL::Block block);

        /// @brief Saves the current image to block that could be written to stream
        static TL::Block Save(Image& image);

        /// @brief Convert the format and size.
        ///
        /// @warning This is a very slow operation
        static TL::Ptr<Image> Convert(Image& image, ImageSize newSize, ImageFormat newFormat);

        /// @brief Default constructor.
        Image() = default;

        Image(Image&& other) = default;
        Image(const Image& other) = delete;
        ~Image();

        Image& operator=(Image&& other) = default;
        Image& operator=(const Image& other) = delete;

        /// @brief Retrieves the name of the texture.
        /// @return The name of the texture.
        const char* GetName() const { return m_name.c_str(); }

        /// @brief Retrieves the format of the texture.
        /// @return The format of the texture.
        ImageFormat GetFormat() const { return m_format; }

        /// @brief Retrieves the size (dimensions) of the texture.
        /// @return The size of the texture.
        ImageSize GetSize() const { return m_size; }

        /// @brief Retrieves the number of mipmap levels in the texture.
        /// @return The number of mipmap levels.
        uint32_t GetMipLevelsCount() const { return m_mipLevelsCount; }

        /// @brief Retrieves the number of array elements in the texture.
        /// @return The number of array elements.
        uint32_t GetArrayElementsCount() const { return m_arrayElementsCount; }

        /// @brief Retrieves the raw binary data of the texture.
        /// @return The binary data of the texture.
        TL::Block GetData() const { return m_data; }

        /// @brief Serializes the texture data into an archive.
        /// @tparam Archive The type of the archive.
        /// @param archive The archive to serialize into.
        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, m_name);
            TL::Encode(archive, (int)m_format);
            TL::Encode(archive, m_size.width);
            TL::Encode(archive, m_size.height);
            TL::Encode(archive, m_size.depth);
            TL::Encode(archive, m_mipLevelsCount);
            TL::Encode(archive, m_arrayElementsCount);
            TL::Encode(archive, m_data);
        }

        /// @brief Deserializes the texture data from an archive.
        /// @tparam Archive The type of the archive.
        /// @param archive The archive to deserialize from.
        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, m_name);
            TL::Decode(archive, (int&)m_format);
            TL::Decode(archive, m_size.width);
            TL::Decode(archive, m_size.height);
            TL::Decode(archive, m_size.depth);
            TL::Decode(archive, m_mipLevelsCount);
            TL::Decode(archive, m_arrayElementsCount);
            TL::Decode(archive, m_data);
        }

    private:
        bool IsDDS() const
        {
            return (uint32_t)m_format > (uint32_t)ImageFormat::BC1;
        }

    private:
        TL::String m_name;             ///< The name of the texture.
        ImageFormat m_format;          ///< The format of the texture.
        ImageSize m_size;              ///< The size (dimensions) of the texture.
        uint32_t m_mipLevelsCount;     ///< The number of mipmap levels in the texture.
        uint32_t m_arrayElementsCount; ///< The number of array elements in the texture.
        TL::Block m_data;              ///< The raw binary data of the texture.
    };

} // namespace Examples::Assets
