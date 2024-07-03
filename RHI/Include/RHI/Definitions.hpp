#pragma once
#include <RHI/Common/Handle.hpp>

#include <cstdint>

namespace RHI
{
    struct Buffer;

    using DeviceMemoryPtr = void*;

    enum class Backend
    {
        Vulkan13,
    };

    enum class DeviceType
    {
        CPU,
        Integerated,
        Dedicated,
        Virtual
    };

    enum class Vendor
    {
        Intel,
        Nvida,
        AMD,
        Other,
    };

    struct Version
    {
        uint16_t major;
        uint16_t minor;
        uint32_t patch;
    };

    struct ApplicationInfo
    {
        const char* applicationName;    // The name of the users application.
        Version     applicationVersion; // The version of the users application.
        const char* engineName;         // The version of the users application.
        Version     engineVersion;      // The version of the users application.
    };

    struct Limits
    {
        size_t stagingMemoryLimit;
    };

    struct StagingBuffer
    {
        DeviceMemoryPtr ptr;
        Handle<Buffer>  buffer;
        size_t          offset;
    };

    // Collection of common definitions

    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };

    // clear values

    struct DepthStencilValue
    {
        float   depthValue;
        uint8_t stencilValue;
    };

    template<typename T>
    struct ColorValue
    {
        T r, g, b, a;
    };

    union ClearValue
    {
        ColorValue<uint8_t>  u8;
        ColorValue<uint16_t> u16;
        ColorValue<uint32_t> u32;
        ColorValue<float>    f32;
        DepthStencilValue    depthStencil;
    };

    // load store operations

    enum class LoadOperation : uint8_t
    {
        DontCare, // The attachment load operation undefined.
        Load,     // Load attachment content.
        Discard,  // Discard attachment content.
    };

    enum class StoreOperation : uint8_t
    {
        DontCare, // Attachment Store operation is undefined
        Store,    // Writes to the attachment are stored
        Discard,  // Writes to the attachment are discarded
    };

    struct LoadStoreOperations
    {
        ClearValue     clearValue;
        LoadOperation  loadOperation;
        StoreOperation storeOperation;
    };

} // namespace RHI