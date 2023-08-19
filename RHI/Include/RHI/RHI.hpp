#pragma once

#include <string>

namespace RHI
{

class Context
{
public:
    Context() = default;
    virtual ~Context() = default;

    uint32_t GetNum() const;
    virtual std::string GetName() const = 0;
};

}