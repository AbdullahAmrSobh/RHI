import ShaderBindingTable;

// Vertex structure
struct VSOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

static const float2 quadVertices[6] = {
    float2( 1.0, -1.0), // bottom right
    float2(-1.0, -1.0), // bottom left
    float2(-1.0,  1.0), // top left

    float2( 1.0,  1.0), // top right
    float2( 1.0, -1.0), // bottom right
    float2(-1.0,  1.0)  // top left
};

[[shader("vertex")]]
VSOutput VSMain(uint vertexId : SV_VertexID)
{
    VSOutput output;
    output.position = float4(quadVertices[vertexId], 0.0, 1.0);
    // Derive UV from NDC: (NDC.xy + 1) * 0.5
    output.uv = quadVertices[vertexId] * 0.5 + 0.5;
    return output;
}

[[shader("pixel")]]
float4 PSMain(VSOutput input) : SV_Target
{
    // Use image fetch instead of sampler
    int2 texSize;
    bindings.pass_lighting_input.GetDimensions(texSize.x, texSize.y);
    int2 texelCoord = int2(input.uv * texSize);
    return bindings.pass_lighting_input.Load(int3(texelCoord, 0));
}
