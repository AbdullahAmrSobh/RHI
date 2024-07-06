#pragma once

#include "RHI/RHI.hpp"

namespace TL = RHI::TL;

template<typename T>
using Handle = RHI::Handle<T>;

template<typename T>
using Ptr = RHI::Ptr<T>;

template<typename T>
using Flags = RHI::Flags<T>;