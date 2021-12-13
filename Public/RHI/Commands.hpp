#pragma once
#include "RHI/Resources.hpp"

namespace RHI {
	
	class ICommandsAllocator {};
	using CommandsAllocatorPtr = Unique<ICommandsAllocator>;
	
}
