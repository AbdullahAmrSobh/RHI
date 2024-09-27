#include "RHI/CommandPool.hpp"
#include "RHI/CommandList.hpp"

namespace RHI
{
    CommandPool::~CommandPool() = default;

    TL::Ptr<CommandList> CommandPool::Allocate(QueueType queueType, CommandListLevel level)
    {
        return std::move(Allocate(queueType, level, 1).front());
    }

} // namespace RHI