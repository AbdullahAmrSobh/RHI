
struct VS_INPUT
{
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

struct DrawData
{
    float4x4 ProjectionMatrix;
    SamplerState sampler0;
    Texture2D texture0;
};

ParameterBlock<DrawData> drawData;

[[shader("vertex")]]
PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(float4(input.pos.xy, 0.f, 1.f), drawData.ProjectionMatrix);
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}

struct GBufferOutput
{
    float4 albedo   : SV_Target0; // Albedo color
};


[[shader("fragment")]]
GBufferOutput PSMain(PS_INPUT input) : SV_Target
{
    let color = input.col * drawData.texture0.Sample(drawData.sampler0, input.uv);
    GBufferOutput output;
    output.albedo = color;
    return output;
}
