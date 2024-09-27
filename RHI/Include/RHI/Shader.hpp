#pragma once

#include "RHI/Export.hpp"

namespace RHI
{
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
