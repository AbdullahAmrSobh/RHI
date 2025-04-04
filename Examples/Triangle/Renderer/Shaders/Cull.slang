#include "Public/GpuScene.h"

RWStructuredBuffer<DrawIndexedParameters> outputBuffer;

[[shader("compute"), numthreads(4, 1, 1)]]
void CSMain(uint3 threadId : SV_DispatchThreadID)
{
    if (threadId.x < 4)
    {
        DrawIndexedParameters params;
        params.indexCount    = 3;
        params.instanceCount = 1;
        params.firstIndex    = 0;
        params.vertexOffset  = 0;
        params.firstInstance = 0;
        outputBuffer[threadId.x] = params;
    }
}