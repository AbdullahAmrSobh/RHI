#pragma once

#include "RHI/Export.hpp"

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
        /// @brief Default constructor.
        ShaderModule();

        /// @brief Virtual destructor.
        virtual ~ShaderModule();
    };
} // namespace RHI
