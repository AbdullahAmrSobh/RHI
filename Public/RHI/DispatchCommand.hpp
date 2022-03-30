#pragma once
#include "RHI/Definitions.hpp"

namespace RHI {

	struct DispatchCommand 
	{
		DispatchCommand() = default;
		
		uint32_t workGroupCountX;
		uint32_t workGroupCountY;
		uint32_t workGroupCountZ;
		
		uint32_t workGroupOffsetX;
		uint32_t workGroupOffsetY;
		uint32_t workGroupOffsetZ;

		
	};

} // namespace RHI
