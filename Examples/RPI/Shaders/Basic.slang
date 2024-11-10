struct VSInput
{
	float3 position;
    float4 color;
};

struct PSInput
{
    float4 csPosition : SV_Position;
    float4 color      : TEXCOORD0;
    float2 uv         : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_Target;
};

struct UniformData
{
    float4x4 transform;
};

struct ShaderParamters
{
	ConstantBuffer<UniformData> viewCB;
    Texture2D<float> texture;
};

ParameterBlock<ShaderParamters> bindGroup0;

[[shader("vertex")]]
PSInput VSMain(VSInput input)
{
    PSInput result;
    result.csPosition = mul(bindGroup0.viewCB.transform, float4(input.position, 1.0f));
    result.color = input.color;
    result.uv = input.position.xy;
    return result;
}

[[shader("fragment")]]
PSOutput PSMain(PSInput input)
{
    uint width, height, levels;
    bindGroup0.texture.GetDimensions(0, width, height, levels);
	PSOutput output;
    output.color = input.color * 0.5;
    output.color += bindGroup0.texture.Load({ uint(input.uv.x * (float)width), uint(input.uv.y * (float)height), 0 }) * 0.5;
	return output;
}
