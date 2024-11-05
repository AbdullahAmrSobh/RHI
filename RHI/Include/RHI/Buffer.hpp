#pragma once

#include "RHI/Format.hpp"
#include "RHI/Handle.hpp"

#include <TL/Flags.hpp>
#include <TL/Utils.hpp>

namespace RHI
{
    RHI_DECLARE_OPAQUE_RESOURCE(Buffer);
    RHI_DECLARE_OPAQUE_RESOURCE(BufferView);

    /// @brief Specifies the types of memory where a buffer can be allocated.
    enum class MemoryType
    {
        CPU,       ///< Buffer allocated on host local memory (GPU access will be slow).
        GPULocal,  ///< Buffer allocated on device local memory (CPU access is not guaranteed).
        GPUShared, ///< Buffer allocated on device local memory (with guaranteed GPU access, used for streaming resources).
    };

    /// @brief Specifies the usage flags for a buffer.
    enum class BufferUsage
    {
        None    = 0 << 0, ///< No usage flags set.
        Storage = 1 << 1, ///< Buffer used for storage operations.
        Uniform = 1 << 2, ///< Buffer used for uniform data.
        Vertex  = 1 << 3, ///< Buffer used for vertex data.
        Index   = 1 << 4, ///< Buffer used for index data.
        CopySrc = 1 << 5, ///< Buffer used as a source in copy operations.
        CopyDst = 1 << 6, ///< Buffer used as a destination in copy operations.
    };

    TL_DEFINE_FLAG_OPERATORS(BufferUsage);

    /// @brief Describes a subregion of a buffer.
    struct BufferSubregion
    {
        size_t      offset = 0; ///< Offset into the buffer.
        size_t      size   = 0; ///< Size of the subregion.

        /// @brief Compares this subregion with another for equality.
        /// @param other The other subregion to compare with.
        /// @return true if both subregions have the same offset and size, false otherwise.
        inline bool operator==(const BufferSubregion& other) const
        {
            return size == other.size && offset == other.offset;
        }
    };

    /// @brief Describes the parameters required to create a buffer.
    struct BufferCreateInfo
    {
        const char*            name       = nullptr;               ///< Name of the buffer.
        MemoryType             heapType   = MemoryType::GPUShared; ///< Memory type for the buffer allocation.
        TL::Flags<BufferUsage> usageFlags = BufferUsage::None;     ///< Usage flags for the buffer.
        size_t                 byteSize   = 0;                     ///< Size of the buffer in bytes.
    };

    /// @brief Describes the parameters required to create a buffer view.
    struct BufferViewCreateInfo
    {
        const char*     name      = nullptr;         ///< Name of the buffer view.
        Handle<Buffer>  buffer    = NullHandle;      ///< Handle to the buffer being viewed.
        Format          format    = Format::Unknown; ///< Format of the buffer view.
        BufferSubregion subregion = {};              ///< Subregion of the buffer being viewed.
    };
} // namespace RHI

namespace std
{
    TL_DEFINE_POD_HASH(RHI::BufferViewCreateInfo);
}
