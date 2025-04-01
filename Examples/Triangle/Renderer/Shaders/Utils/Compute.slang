#include "Public/GpuScene.h"
    struct AABB
    {
        F32_3 min;
        F32_3 max;
    };

    struct BatchDrawRequest
    {
        uint32_t transformID; // offset into the per-instance buffer containg transforms!
        uint32_t transformsCount;
        uint32_t geometryID;
        uint32_t materialID;
    };

    struct DrawArguments
    {
        uint32_t elementsCount;
        uint32_t indexOffset;
        uint32_t vertexOffset;
        AABB     aabb;
    };


struct BindGroup
{
    uint32_t drawCount;
    float4 frustum[4];

    StructuredBuffer<BatchDrawRequest> drawRequests;
    StructuredBuffer<DrawArguments> drawArguments;
    RWStructuredBuffer<uint32_t>              outIndirectDrawCount;
    AppendStructuredBuffer<DrawIndexedParameters> outIndirectDrawIndexedParameters;
};

ParameterBlock<BindGroup> bindGroup;

bool CheckAABB(float4 frustum[4], AABB aabb)
{
    return true;
}

[[shader("compute")]]
void CSMain(uint32_t3 input)
{
    for (uint32_t i = input.x; i < bindGroup.drawCount; i++)
    {
        var request = bindGroup.drawRequests[i];

        var args = bindGroup.drawArguments[request.geometryID];

        if (CheckAABB(bindGroup.frustum, args.aabb) == false)
        {
            continue;
        }

        DrawIndexedParameters parameters;
        parameters.indexCount = args.elementsCount;
        parameters.instanceCount = request.transformsCount;
        parameters.firstIndex    = args.indexOffset;
        parameters.vertexOffset = args.vertexOffset;
        parameters.firstInstance = request.transformID;
        bindGroup.outIndirectDrawIndexedParameters.Append(parameters);
    }
}

