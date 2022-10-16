#pragma once
#include "RHI/Resource.hpp"

#include <type_traits>

namespace RHI
{

namespace internal
{

    template <typename T>
    struct ConvertResourceToView
    {
        static_assert(std::is_same_v<T, IImage> || std::is_same_v<T, IBuffer>, "Invalid Resource Type");
    };

    template <>
    struct ConvertResourceToView<IImage>
    {
        using Type = IImageView;
    };

    template <>
    struct ConvertResourceToView<IBuffer>
    {
        using Type = IBufferView;
    };

    template <typename T>
    struct ConvertResourceToDesc
    {
        static_assert(std::is_same_v<T, IImage> || std::is_same_v<T, IBuffer>, "Invalid Resource Type");
    };

    template <>
    struct ConvertResourceToDesc<IImage>
    {
        using Type = ImageDesc;
    };

    template <>
    struct ConvertResourceToDesc<IBuffer>
    {
        using Type = BufferDesc;
    };

    template <typename T>
    struct ConvertViewToResource
    {
        static_assert(std::is_same_v<T, IImageView> || std::is_same_v<T, IBufferView>, "Invalid Resource Type");
    };

    template <>
    struct ConvertViewToResource<IImageView>
    {
        using Type = IImage;
    };

    template <>
    struct ConvertViewToResource<IBufferView>
    {
        using Type = IBuffer;
    };

    template <typename T>
    struct ConvertViewToDesc
    {
        static_assert(std::is_same_v<T, IImageView> || std::is_same_v<T, IBufferView>, "Invalid Resource Type");
    };

    template <>
    struct ConvertViewToDesc<IImageView>
    {
        using Type = ImageViewDesc;
    };

    template <>
    struct ConvertViewToDesc<IBufferView>
    {
        using Type = BufferViewDesc;
    };

} // namespace internal

template <typename T>
using ConvertResourceToView = typename internal::ConvertResourceToView<T>::Type;

template <typename T>
using ConvertResourceToDesc = typename internal::ConvertResourceToDesc<T>::Type;

template <typename T>
using ConvertViewToResource = typename internal::ConvertViewToResource<T>::Type;

template <typename T>
using ConvertViewToDesc = typename internal::ConvertViewToDesc<T>::Type;

// asserts the ensure that the type traits work correctly. 
static_assert(std::is_same_v<ConvertResourceToView<IImage>, IImageView>, "Invalid Conversion");
static_assert(std::is_same_v<ConvertResourceToView<IBuffer>, IBufferView>, "Invalid Conversion");
static_assert(std::is_same_v<ConvertResourceToDesc<IImage>, ImageDesc>, "Invalid Conversion");
static_assert(std::is_same_v<ConvertResourceToDesc<IBuffer>, BufferDesc>, "Invalid Conversion");
static_assert(std::is_same_v<ConvertViewToResource<IImageView>, IImage>, "Invalid Conversion");
static_assert(std::is_same_v<ConvertViewToResource<IBufferView>, IBuffer>, "Invalid Conversion");
static_assert(std::is_same_v<ConvertViewToDesc<IImageView>, ImageViewDesc>, "Invalid Conversion");
static_assert(std::is_same_v<ConvertViewToDesc<IBufferView>, BufferViewDesc>, "Invalid Conversion");

} // namespace RHI