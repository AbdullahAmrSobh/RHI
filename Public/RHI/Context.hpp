#pragma once

#include "RHI/Definitions.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{
	
	class ICommandList;
	class IContext
	{
	public:
		virtual ~IContext() = default;
		
		virtual EResultCode Present(uint32_t _count, ISwapChain** _ppSwapChains) = 0; 

	};
	using ContextPtr = Unique<IContext>;

}
