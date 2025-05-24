// import Public.GpuScene;

struct UGBVertexInput
{
    float3 position;
    float3 normal;
    float2 uv;
    float4x4 transform;
};

struct GBufferInput
{
    float4 csPosition;
    float3 wsPosition;
    float3 normal;
    float2 uv;
};

struct GBufferOutput
{
    float4 wsPosition; // World position
    float4 normal;     // Normal vector
    float2 material;   // (UV + material ID + roughness + metallic)
};

[[shader("vertex")]]
GBufferInput VSMain(UGBVertexInput input)
{
    GBufferInput output;
    return output;
}

[[shader("fragment")]]
GBufferOutput PSMain(GBufferInput input)
{
    GBufferOutput output;
    output.wsPosition = float4(input.wsPosition, 1.0f);
    output.normal = float4(input.normal, 1.0f);
    output.material = input.uv;
    return output;
}
