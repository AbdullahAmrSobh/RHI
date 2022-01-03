#pragma once
#include "RHI/Definitions.hpp"

namespace RHI {
	
	class ICommandsAllocator {};
	using CommandsAllocatorPtr = Unique<ICommandsAllocator>;
	
	class ICommandList {};
	using CommandListPtr = Unique<ICommandList>;
}
