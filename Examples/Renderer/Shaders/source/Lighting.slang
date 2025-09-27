#include <Shaders/GpuCommonStructs.h>

struct LightingPassParams
{
    uint32_t lightsCount;

    StructuredBuffer<GPU::DirectionalLight> dirLight;

    Texture2D wsPosition;
    Texture2D normal;
    Texture2D texcoord;
    Texture2D depth;

    RWTexture2D outColor;
};

ParameterBlock<LightingPassParams> params;
[[shader("compute"), numthreads(32, 32, 1)]]
void CSMain(uint3 threadId : SV_DispatchThreadID)
{
    uint2 coord = threadId.xy;

    // Sample GBuffer
    float3 wsPos = params.wsPosition.Load(int3(coord, 0)).xyz;
    float3 normal = normalize(params.normal.Load(int3(coord, 0)).xyz);

    float3 litColor = float3(0.0, 0.0, 0.0);

    // Iterate over all directional lights
    for (uint i = 0; i < params.lightsCount; ++i)
    {
        let light = params.dirLight[i];
        float3 lightDir = normalize(light.direction.xyz);
        float3 lightColor = light.color.rgb;
        float intensity = max(dot(normal, -lightDir), 0.0);

        litColor += lightColor * intensity;
    }

    // Output
    params.outColor.Store(coord, float4(litColor, 1.0));
}