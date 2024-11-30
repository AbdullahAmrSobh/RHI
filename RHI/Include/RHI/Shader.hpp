#pragma once

#include "RHI/Export.hpp"
#include "RHI/Common.hpp"

#include <TL/Span.hpp>

namespace RHI
{
    struct ShaderModuleCreateInfo
    {
        const char*              name = nullptr;
        TL::Span<const uint32_t> code = {};
    };

    /// @brief Represents a shader module used in pipeline creation.
    class RHI_EXPORT ShaderModule
    {
    public:
        RHI_INTERFACE_BOILERPLATE(ShaderModule);
    };
} // namespace RHI
