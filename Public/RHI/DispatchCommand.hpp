#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

class IDescriptorSet;
class IPipelineLayout;
class IPipelineState;

struct DispatchCommand
{
    DispatchCommand() = default;

	inline void SetPipelineState(IPipelineState& pipelineState)
	{
		pPipelineState = &pipelineState;
	}
    
	inline void SetDescriptorSets(IPipelineLayout& layout, const std::vector<IDescriptorSet*>& descriptorSets)
    {
        pPipelineLayout   = &layout;
        descriptorSetPtrs = descriptorSets;
    }

    IPipelineState*              pPipelineState;
    IPipelineLayout*             pPipelineLayout;
    std::vector<IDescriptorSet*> descriptorSetPtrs;
    
    uint32_t workGroupCountX  = 0;
    uint32_t workGroupCountY  = 0;
    uint32_t workGroupCountZ  = 0;
    uint32_t workGroupOffsetX = 0;
    uint32_t workGroupOffsetY = 0;
    uint32_t workGroupOffsetZ = 0;
};

} // namespace RHI
