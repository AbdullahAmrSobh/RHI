#pragma once

namespace RHI
{

enum class ResultCode
{
    Success,
    Fail,
    Timeout,
    NotReady,
    HostOutOfMemory,
    DeviceOutOfMemory,
    ExtensionNotAvailable,
    InvalidArguments,
    FeatureNotAvailable,
    InvalidObject,
};

}  // namespace RHI