#pragma once

namespace RHI
{

enum class SampleCount
{
    None    = 0 << 0,
    Samples1  = 1 << 0, 
    Samples2  = 1 << 1,
    Samples4  = 1 << 2,
    Samples8  = 1 << 3,
    Samples16 = 1 << 4,
    Samples32 = 1 << 5,
    Samples64 = 1 << 6,
};

}