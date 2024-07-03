#pragma once

#include <RHI/RHI.hpp>

#include <fstream>

namespace TL = RHI::TL;

template<typename T>
using Handle = RHI::Handle<T>;

template<typename T>
using Ptr = RHI::Ptr<T>;

inline static TL::Vector<uint32_t> ReadBinaryFile(std::string_view filePath)
{
    std::ifstream file(filePath.data(), std::ios::binary | std::ios::ate);
    RHI_ASSERT(file.is_open());

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    RHI_ASSERT(size % 4 == 0);

    TL::Vector<uint32_t> spirv(size / 4);
    RHI_ASSERT(file.read(reinterpret_cast<char*>(spirv.data()), size));

    return spirv;
}
