#pragma once

namespace RHI
{
    enum class Access
    {
        None      = 0,
        Read      = 1 << 0,
        Write     = 1 << 1,
        ReadWrite = Read | Write,
    };

    inline static bool IsWriteAccess(Access access)
    {
        return access == Access::ReadWrite || access == Access::Write;
    }
} // namespace RHI