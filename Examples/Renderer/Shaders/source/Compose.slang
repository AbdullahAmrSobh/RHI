struct LightingInputs
{
    Texture2D wsPosition;
    Texture2D normal;
    Texture2D material;
    Texture2D depth;
    RWTexture2D color;
};

[[shader("compute"), numthreads(32, 32, 1)]]
void CSMain(
    uint3 dispatchThreadID : SV_DispatchThreadID,
    uint3 groupThreadID    : SV_GroupThreadID,
    uint3 groupID          : SV_GroupID,
    ParameterBlock<LightingInputs> m_params,
)
{
}
