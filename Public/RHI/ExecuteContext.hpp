#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

enum class ERenderPassQueueType;
class ICommandList;

class ExecuteContext
{
public:
    ExecuteContext(Span<ICommandList*> commandLists)
		: m_commandLists(commandLists.begin(), commandLists.end())
	{
	}
    
    inline ERenderPassQueueType GetQueueType() const
    {
        return m_queueType;
    }
    
    inline uint32_t GetCommandListCount() const
    {
        return CountElements(m_commandLists);
    }
    
		
    inline Span<ICommandList*> GetCommandLists()
    {
        return {m_commandLists.begin(), m_commandLists.end()};
    }

private:
    ERenderPassQueueType       m_queueType;
    std::vector<ICommandList*> m_commandLists;
};

} // namespace RHI
