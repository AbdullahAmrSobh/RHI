#pragma once

#include <memory>
#include <span>
#include <string>
#include <vector>

#include "RHI/Export.hpp"
#include "RHI/Flags.hpp"
#include "RHI/Object.hpp"

namespace RHI
{

class Context;

enum class QueueType
{
    Graphics,
    Compute,
    Transfer,
};

struct QueueInfo
{
    QueueType type;
    uint32_t  id;
};

struct PassCreateInfo
{
    std::string name;
    QueueType   type;
};

/// @brief Represents a pass, which encapsulates a GPU task.
class RHI_EXPORT Pass
{
public:
    virtual ~Pass() = default;
    
};

}  // namespace RHI