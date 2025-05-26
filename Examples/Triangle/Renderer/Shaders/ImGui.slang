struct VSInput
{
    float2 pos;
    float2 uv;
    float4 col;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

ConstantBuffer<float4x4> projection;
SamplerState             sampler0;
Texture2D                texture0;

[[shader("vertex")]]
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.pos = mul(float4(input.pos.xy, 0.f, 1.f), projection);
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}

[[shader("fragment")]]
float4 PSMain(PSInput input) : SV_Target0
{
    let color = input.col * texture0.Sample(sampler0, input.uv);
    return color;
}
