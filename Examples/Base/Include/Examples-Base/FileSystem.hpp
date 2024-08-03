#pragma once

#include "Examples-Base/Common.hpp"

#include <fstream>

namespace Examples
{
    inline static TL2::Vector<uint32_t> ReadBinaryFile(std::string_view filePath)
    {
        std::ifstream file(filePath.data(), std::ios::binary | std::ios::ate);
        TL_ASSERT(file.is_open()); // "Failed to open SPIR-V file: " + filePath

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        TL_ASSERT(size % 4 == 0); // "Invalid SPIR-V file size: " + filePath

        TL2::Vector<uint32_t> spirv(size / 4);
        TL_ASSERT(file.read(reinterpret_cast<char*>(spirv.data()), size)); // "Failed to read SPIR-V file: " + filePath

        return spirv;
    }
} // namespace Examples